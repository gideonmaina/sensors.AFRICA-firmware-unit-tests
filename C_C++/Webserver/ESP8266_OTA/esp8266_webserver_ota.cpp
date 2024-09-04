
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h
ESP8266WebServer server(80);

constexpr unsigned LARGE_STR = 512 - 1;
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
    server.send(200, FPSTR(TXT_CONTENT_TYPE_TEXT_HTML), s);

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
    server.on(F("/ota_begin"), HTTP_POST, webserver_ota_begin);

    Serial.print("Starting Webserver... "),
        Serial.println(WiFi.localIP().toString());
    server.begin();
}

static void webserver_ota_upload()
{

    server.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
    server.sendHeader(F("Pragma"), F("no-cache"));
    server.sendHeader(F("Expires"), F("0"));

    RESERVE_STRING(page_content, XLARGE_STR);
    // start_html_page(page_content, emptyString);
    start_html_page(page_content, "OTA update");
    page_content += FPSTR(WEB_B_BR);
    page_content += F("<form method='POST' action='/ota_begin' enctype='multipart/form-data' style='width:100%;'>\n<b> OTA OVER ESP AP WEBSERVER </b><br/>");
    page_content += F("<b> Firmware Loader bin</b><br/>");
    String form_input = F("<label for='loader_checksum'><input type='text' name='loader_checksum' placeholder='Enter loader checksum'><br/>");
    form_input += F("<label for='loader'><input type='file' name='loader' accept='.bin'><br/>");
    page_content += form_input;
    server.sendContent(page_content);
    page_content = "";
    form_input = "";
    page_content += F("<b> Firmware Bin</b><br/>");
    form_input += F("<label for='fmw_checksum'><input type='text' name='fmw_checksum'  placeholder='Enter firmware checksum'><br/>");
    form_input += F("<label for='firmware'><input type='file' name='firmware' accept='.bin'><br/>");
    page_content += form_input;
    page_content += F("<br/><br/><br/><input  type='submit' value='Upload'>");
    end_html_page(page_content);
}

static void webserver_ota_begin()
{
}