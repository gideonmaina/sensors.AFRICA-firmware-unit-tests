/************************************************************************
 * This Javascript code can be used to test multiple file uploads.
 * The url-encoded and binary file data are sent esparately.
 * Use Minify VS Code to minimize the size and put inside a script tag
 * in the ESP html upload page
 ***********************************************************************/

const form = document.querySelector("form");
form.addEventListener("submit", handleSubmit);

function handleSubmit(event) {
  event.preventDefault();
  event.stopPropagation();
  console.log("Sending form data");

  const checksum_url = "http://192.168.4.1/parse_checksum";
  const upload_url = "http://192.168.4.1/ota_upload";
  const method = "post";

  let xhr = new XMLHttpRequest();

  // callback to handle the server response
  xhr.onreadystatechange = function () {
    if (xhr.readyState === 4 && xhr.status === 200) {
      // The file has been uploaded successfully
      document.createElement("p").innerText = xhr.responseText;
      console.log("Send successfull");
    }
  };

  const data = new FormData(form);
  // for (const [key, value] of data) {
  //   console.log(`${key}: ${value}`);
  // }

  //   first send the checksums
  xhr.open(method, checksum_url);
  xhr.setRequestHeader("Accept", "*/*");
  //   xhr.setRequestHeader("Access-Control-Allow-Origin", "*");
  xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

  const checksums = `loader_checksum=${data.get(
    "loader_checksum"
  )}&fmw_checksum=${data.get("fmw_checksum")}`;
  console.log(checksums);

  xhr.send(checksums);

  xhr = new XMLHttpRequest();
  const files = new FormData();
  files.append("loader", data.get("loader"));
  files.append("firmware", data.get("firmware"));
  files.values().forEach((element) => {
    console.log(element);
  });
  xhr.open(method, upload_url);
  xhr.setRequestHeader("Accept", "*/*");
  xhr.setRequestHeader("Access-Control-Allow-Origin", "*");

  xhr.send(files);
}
