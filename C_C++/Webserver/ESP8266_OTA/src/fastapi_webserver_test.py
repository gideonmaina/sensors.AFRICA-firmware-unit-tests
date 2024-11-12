from fastapi import FastAPI, UploadFile, Request
from fastapi.templating import Jinja2Templates
from fastapi.responses import HTMLResponse, JSONResponse

app = FastAPI()

templates = Jinja2Templates(directory="templates")


@app.get("/", response_class=HTMLResponse)
def root():
    html_content = """
    <html>
        <head>
            <title>Fast API backend</title>
        </head>
        <body>
            <h1>Upload testing using Fast API</h1>
            <a href="/ota_update"> Go to upload page</a>
        </body>
    </html>
    """

    return HTMLResponse(html_content, status_code=200)


@app.get("/ota_update", response_class=HTMLResponse)
async def renderOTAuploadpage(request: Request):
    return templates.TemplateResponse("upload-page.html", {"request": request})


@app.post("/ota_upload")
async def uploadfile(firmware: UploadFile):
    try:
        print("saving file....")
        # file_path = f"./{firmware.filename}"
        file_path = f"./mynewestfirmware.bin"
        print(file_path)
        # return {"message": "File saved successfully"}
        # return HTMLResponse(
        #     """
        #             <p>File Saved Successfully!</p>
        #             <p>Attempting firmware update...</p>"""
        # )w s
        with open(file_path, "wb") as f:
            f.write(firmware.file.read())
            return {"message": "File saved successfully"}
    except Exception as e:
        return {"message": e.args}