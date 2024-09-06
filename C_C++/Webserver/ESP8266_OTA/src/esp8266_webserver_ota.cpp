
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>
ESP8266WebServer server(80);

constexpr unsigned LARGE_STR = 512 - 1;
constexpr unsigned XLARGE_STR = 1024 - 1;

String loader_checksum, firmware_checksum, fname = "";
// Replace bin filenames with the exactly the ones you want to upload
String loaderfilename = "/loader-002.bin";
String new_firmware_filename = "/new_firmware.bin";
bool loader_bin_saved = false;
bool firmware_bin_saved = false;

File uploadFile; // a File object to temporarily store the received

// Function declarations
void log_args();
void uploadFiles();
static void webserver_ota_begin();
static void webserver_ota_upload();
static void setup_webserver();
static void file_checker();
void two_stage_firmware_update();
bool validate_bin_md5(String filename, String md5_checksum);
static bool SPIFFSAutoUpdate(String newFirmware, String newMD5);

#define RESERVE_STRING(name, size)      \
    String name((const char *)nullptr); \
    name.reserve(size)

/****************************************************************
 * HTML helpers pulled from openstuttgart
 * Ignore as not main part of this test
 **************************************************************/

const char WEB_PAGE_HEADER[] PROGMEM = "<!DOCTYPE html><html>\
<head>\
<title>{t}</title>";

const char WEB_PAGE_HEADER_HEAD[] PROGMEM = "<meta name='viewport' content='width=device-width'>\
<style type='text/css'>\
body{font-family:Arial;margin:0}\
.content{margin:10px}\
.r{text-align:right}\
td{vertical-align:top;}\
a{text-decoration:none;padding:10px;background:#3ba;color:white;display:block;width:auto;border-radius:5px;box-shadow:0px 2px 2px #3ba;}\
.wifi{background:none;color:blue;padding:5px;display:inline;}\
input[type='text']{width:100%;}\
input[type='password']{width:100%;}\
input[type='submit']{color:white;text-align:left;border-radius:5px;font-size:medium;background:#b33;box-shadow:0px 2px 2px #b33;padding:9px !important;width:100%;border-style:none;}\
input[type='submit']:hover {background:#d44} \
.s_green{padding:9px !important;width:100%;border-style:none;background:#3ba;color:white;text-align:left;}\
</style>\
</head><body>\
<div style='min-height:129px;background-color:#3ba;margin-bottom:20px;box-shadow:0px 4px 6px #3ba'>";

const char WEB_PAGE_FOOTER[] PROGMEM = "<br/><br/>"
                                       "<br/><br/><br/>"
                                       "</div></body></html>\r\n";

static void start_html_page(String &page_content, const String &title)
{

    RESERVE_STRING(s, LARGE_STR);
    s = FPSTR(WEB_PAGE_HEADER);
    s.replace("{t}", title);
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, FPSTR("text/html; charset=utf-8"), s);

    server.sendContent_P(WEB_PAGE_HEADER_HEAD);
    s.replace("{t}", title);
    if (title != " ")
    {
        s.replace("{n}", F("&raquo;"));
    }
    else
    {
        s.replace("{n}", "");
    }
    page_content += s;
}

static void end_html_page(String &page_content)
{
    if (page_content.length())
    {
        server.sendContent(page_content);
    }
    server.sendContent_P(WEB_PAGE_FOOTER);
}

/************************************************************************************************
 * WEBSERVER SET UP
 *
 ************************************************************************************************/

static void setup_webserver()
{
    server.on(F("/ota_upload"), webserver_ota_upload);
    server.on(F("/ota_begin"), HTTP_POST, []()
              { 
                // server.sendHeader(F("Transfer-Encoding"), F("chunked"));
                server.send(200); }, // Send status 200 (OK) to tell the client we are ready to receive
              webserver_ota_begin);

    Serial.print("Starting Webserver... ");
    server.begin();
}

static void webserver_ota_upload()
{

    RESERVE_STRING(page_content, XLARGE_STR);
    start_html_page(page_content, "OTA update");
    page_content += "<br/><br/>";
    page_content += F("<form method='POST' action='/ota_begin' enctype='multipart/form-data' style='width:100%;'>\n<b> OTA OVER ESP AP WEBSERVER </b><br/>");
    page_content += F("<b> Firmware Loader bin</b><br/>");
    String form_input = F("<div><label for='loader_checksum'><input type='text' name='loader_checksum' placeholder='Enter loader checksum'><br/>");
    form_input += F("<label for='loader'><input type='file' name='loader' accept='.bin'></div><br/>");
    page_content += form_input;
    server.sendContent(page_content);
    page_content = "";
    form_input = "";
    page_content += F("<b> Firmware Bin</b><br/>");
    form_input += F("<div><label for='fmw_checksum'><input type='text' name='fmw_checksum'  placeholder='Enter firmware checksum'><br/>");
    form_input += F("<label for='firmware'><input type='file' name='firmware' accept='.bin'></div><br/>");
    page_content += form_input;
    page_content += F("<br/><br/><br/><div><input  type='submit' value='Upload'></div>");
    end_html_page(page_content);
}

static void webserver_ota_begin()
{
    log_args();
    if (server.args() > 0)
    {
        // Debuging: Log args
        log_args();

        // get server post arguements
        if (!server.hasArg("loader_checksum"))
        {
            Serial.println("Loader bin MD5 checksum missing");
            return;
        }
        loader_checksum = server.arg("loader_checksum");
        if (!server.hasArg("fmw_checksum"))
        {
            Serial.println("Firmware bin MD5 checksum missing");
            return;
        }
        firmware_checksum = server.arg("fmw_checksum");
    }

    // uploadFiles();
    uploadFiles();
}

void uploadFiles()
{
    // upload a new file to the SPIFFS
    HTTPUpload &upload = server.upload();

    Serial.print("Server upload status: ");
    Serial.println(upload.status);

    if (upload.status == UPLOAD_FILE_START)
    {

        fname = upload.filename;
        if (!fname.startsWith("/"))
            fname = "/" + fname;
        Serial.print("Upload File Name: ");
        Serial.println(fname);
        uploadFile = SPIFFS.open(fname, "w"); // Open the file for writing in SPIFFS (create if it doesn't exist)
        if (uploadFile)
        {
            Serial.println("File opened");
        }
        // fname = String();
        Serial.print("fname: ");
        Serial.println(fname);
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (uploadFile)
        {
            uploadFile.write(upload.buf, upload.currentSize);
            Serial.println("written");
        }
    }

    else if (upload.status == UPLOAD_FILE_END)
    {
        if (uploadFile)
        {                       // If the file was successfully created
            uploadFile.close(); // Close the file again
            Serial.print("File Upload Size: ");
            Serial.println(upload.totalSize);
            String msg = "201: Successfully created file ";
            msg += fname;
            server.send(200, "text/plain", msg);
            Serial.println(msg);

            if (fname = loaderfilename)
            {
                loader_bin_saved = true;
            }
            else if (fname = new_firmware_filename)
            {
                firmware_bin_saved = true;
            }
        }
        else
        {
            String err_msg = "500: failed creating file ";
            err_msg += fname;
            server.send(500, "text/plain", err_msg);
            Serial.println(err_msg);
        }
    }
}

void log_args()
{

    Serial.print("Recieved args: ");
    Serial.println(server.args());

    for (int i = 0; i < server.args(); i++)
    {

        String log = "Arg ";
        log += i;
        log += ": ";
        log += server.argName(i);
        Serial.println(log);
    }
}

static void file_checker()
{
    File loaderFile = SPIFFS.open(F("/loader-002.bin"), "r");
    if (!loaderFile)
    {
        Serial.println("loader file does not exist");
    }
    else
    {

        Serial.println("loader file present");
        loaderFile.close();
    }

    File fmwFile = SPIFFS.open(F("/new_firmware.bin"), "r");

    if (!fmwFile)
    {
        Serial.println("firmware file does not exist");
    }
    else
    {

        Serial.println("firmware file present");
        fmwFile.close();
    }
}

void setup()
{
    delay(1000);
    Serial.begin(9600);
    delay(2000);
    const char *ssid = "ESP8266-OTA-TEST";
    const char *password = "123456789";
    WiFi.mode(WIFI_AP);
    const IPAddress apIP(192, 168, 4, 1);
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    delay(2000);
    setup_webserver();
    SPIFFS.begin();
    delay(2000);
    file_checker(); // Reset after successful file upload to view saved files
}

void loop()
{
    server.handleClient();
    if (loader_bin_saved && firmware_bin_saved)
    {
        Serial.println("Beginning two stage firmware update");
        void two_stage_firmware_update();
    }
}

void firmware_update()
{

    // validate new firmware file md5

    if (!validate_bin_md5(loaderfilename, loader_checksum)) // This is actually not needed for single stage
    {
        Serial.print('Deleting file: ');
        Serial.println(loaderfilename);
        SPIFFS.remove(loaderfilename);
        loader_bin_saved = false;
        Serial.println("Two-stage firmware update failed at md5 checksum validation");
        return;
    }
    if (!validate_bin_md5(new_firmware_filename, firmware_checksum))
    {
        Serial.print('Deleting file: ');
        Serial.println(new_firmware_filename);
        SPIFFS.remove(new_firmware_filename);
        firmware_bin_saved = false;
        Serial.println("Two-stage firmware update failed at md5 checksum validation");
        return;
    }

    // begin update

    if (!SPIFFSAutoUpdate(new_firmware_filename, firmware_checksum))
    {
        Serial.println("SPIFFS auto update failed. Deleting files");

        SPIFFS.remove(loaderfilename);
        SPIFFS.remove(new_firmware_filename);

        loader_bin_saved = false;
        firmware_bin_saved - false;
    }
    else
    {
        Serial.println("System updated with new firmware successfully!");
        // Either way delete those files
        SPIFFS.remove(loaderfilename);
        SPIFFS.remove(new_firmware_filename);

        loader_bin_saved = false;
        firmware_bin_saved - false;
    }
}

bool validate_bin_md5(String filename, String md5_checksum)
{

    // validate new firmware file md5
    MD5Builder md5;
    String md5String = ""; // initialize to invalidate if file fails to open
    size_t file_size = -1; // initialize to invalidate if file fails to open
    uploadFile = SPIFFS.open(filename, "r");
    if (uploadFile)
    {

        uploadFile.size();
        md5.begin();
        md5.addStream(uploadFile, file_size);
        md5.calculate();
        uploadFile.close();
        md5String = md5.toString();
    }

    // Firmware is always at least 128 kB and padded to 16 bytes
    if (file_size < (1 << 17) || (file_size % 16 != 0) || md5_checksum != md5String)
    {
        Serial.print("M5 validation failed for file ");
        Serial.print(filename);
        return false;
    }
    else
    {
        Serial.print("M5 validation passed for file ");
        Serial.print(filename);
        return true;
    }
}

static bool SPIFFSAutoUpdate(String newFirmware, String newMD5)
{

    if (!SPIFFS.exists(newFirmware))
    {
        Serial.print("No Firmware file found, looking for: ");
        Serial.println(newFirmware);
        return false;
    }
    File updateFile = SPIFFS.open(newFirmware, "r");
    if (!updateFile)
    {
        Serial.print("Failed to open : ");
        Serial.print(newFirmware);
        return false;
    }
    if (updateFile.size() >= ESP.getFreeSketchSpace())
    {
        Serial.println("Cannot update, Firmware too large");
        return false;
    }
    if (!Update.begin(updateFile.size(), U_FLASH))
    {
        StreamString error;
        Update.printError(error);

        Serial.print("Update.begin returned: "),
            Serial.println(error);
        return false;
    }

    // set MD5
    Update.setMD5(newMD5.c_str());

    if (Update.writeStream(updateFile) != updateFile.size())
    {
        StreamString error;
        Update.printError(error);

        Serial.print("Update.writeStream returned: ");
        Serial.print(error);
        return false;
    }
    updateFile.close();

    if (!Update.end())
    {
        StreamString error;
        Update.printError(error);

        Serial.println("Update.end() returned: ");
        Serial.print(error);
        return false;
    }

    Serial.println("Erasing SDK config.");
    ESP.eraseConfig();

    Serial.println("Finished successfully.. Rebooting!");
    delay(500);
    ESP.restart();
    return true;
}
