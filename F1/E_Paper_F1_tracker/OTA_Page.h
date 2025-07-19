#pragma once
#include <WebServer.h>
#include <Preferences.h>

extern WebServer    server;
extern Preferences  preferences;

void handleOTAUpdatePage() {
      Serial.println("HTTP GET /  OTA page");

    preferences.begin("ota", true);
  String lastFw = preferences.getString("Last File Uploaded: ", "N/A Upload by Arduino IDE");
  preferences.end();
  String html = R"rawliteral(

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=0.8">
    <title>OTA Update</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            text-align: center;
            margin: 0;
            padding: 40px 20px;
            background-color: #f0f2f5; 
            color: #333;
            display: flex;
            flex-direction: column;
            align-items: center;
            min-height: 100vh;
        }

        h1 {
            color: #2c3e50; 
            margin-bottom: 30px;
            font-size: 2.2em;
        }

        .container {
            background-color: #ffffff;
            padding: 40px;
            border-radius: 12px;
            box-shadow: 0 8px 20px rgba(0, 0, 0, 0.1);
            width: 100%;
            max-width: 600px;
            box-sizing: border-box;
        }

        #drop-area {
            border: 3px dashed #a0aec0; 
            background-color: #f7fafc; 
            padding: 60px 30px;
            margin: 30px auto;
            border-radius: 10px;
            cursor: pointer;
            transition: all 0.3s ease;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
            min-height: 150px;
        }

        #drop-area.highlight {
            border-color: #4CAF50; 
            background-color: #e6ffe6; 
            box-shadow: 0 0 15px rgba(76, 175, 80, 0.3);
        }

        #drop-area p {
            margin: 0;
            font-size: 1.1em;
            color: #666;
            margin-top: 15px; 
        }

        #drop-area svg {
            width: 50px;
            height: 50px;
            fill: #a0aec0;
            transition: fill 0.3s ease;
        }

        #drop-area.highlight svg {
            fill: #4CAF50;
        }

        .button-group {
            display: inline;
            flex-direction: column;
            gap: 15px;
            margin-top: 30px;
        }
          .back-button-group {
            display: flex;
            flex-direction: vertical;
            gap: 15px;
            margin-top: 10px;
        }

        button {
            padding: 12px 25px;
            border: none;
            border-radius: 8px;
            font-size: 1em;
            cursor: pointer;
            transition: background-color 0.3s ease, transform 0.2s ease;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        }

        #uploadButton {
            background-color: #007bff; /*
            color: white;
        }

        #uploadButton:hover {
            background-color: #0056b3;
            transform: translateY(-2px);
        }

        #uploadButton:disabled {
            background-color: #cccccc;
            cursor: not-allowed;
        }

        .go-back-button {
      background: #007bff;
      color: #ffffff;
      border: none;
      padding: 12px;
      display: inline;
      width: 100%;
      border-radius: 8px;
      font-size: 1rem;
      font-weight: 600;
      cursor: pointer;
      box-shadow: 0 2px 10px var(--glow-color);
      transition: transform 0.3s, box-shadow 0.3s;
        }

        .go-back-button:hover {
               transform: scale(1.03);
      box-shadow: 0 4px 15px var(--glow-color);
        }

        #progress-bar {
            width: 100%;
            background-color: #e9ecef; 
            border-radius: 5px;
            margin: 25px auto 15px;
            overflow: hidden;
            height: 25px;
            box-shadow: inset 0 1px 3px rgba(0, 0, 0, 0.1);
        }

        #progress {
            width: 0%;
            height: 100%;
            background-color: #28a745; 
            border-radius: 5px;
            text-align: center;
            line-height: 25px;
            color: white;
            transition: width 0.3s ease;
        }

        #status {
            margin-top: 15px;
            font-size: 1em;
            color: #555;
            min-height: 20px; 
        }

        h4 {
            color: #2c3e50;
            margin-top: 40px;
            font-size: 1.1em;
        }

        hr {
            border: 0;
            height: 1px;
            background-image: linear-gradient(to right, rgba(0, 0, 0, 0), rgba(0, 0, 0, 0.15), rgba(0, 0, 0, 0));
            width: 80%;
            margin: 40px auto;
        }
    </style>
</head>
<body>
    <h1>F1 Tracker OTA</h1>
   <p><strong>Current firmware:</strong> )rawliteral"
      + lastFw +
    R"rawliteral(</p>
    <div class="container">
        <div id='drop-area'>
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24"><path d="M14 2H6C4.89543 2 4 2.89543 4 4V20C4 21.1046 4.89543 22 6 22H18C19.1046 22 20 21.1046 20 20V8L14 2ZM18 20H6V4H13V9H18V20ZM12 11C10.3431 11 9 12.3431 9 14C9 15.6569 10.3431 17 12 17C13.6569 17 15 15.6569 15 14C15 12.3431 13.6569 11 12 11ZM12 13C12.5523 13 13 13.4477 13 14C13 14.5523 12.5523 15 12 15C11.4477 15 11 14.5523 11 14C11 13.4477 11.4477 13 12 13ZM12 18C9.23858 18 7 15.7614 7 13C7 10.2386 9.23858 8 12 8C14.7614 8 17 10.2386 17 13C17 15.7614 14.7614 18 12 18Z"></path></svg>
            <p>Drag & Drop your firmware file here, or click to select</p>
            <input type='file' id='fileInput' name='update' hidden>
        </div>
        
        <div class="button-group">
            <button id='uploadButton' disabled>Upload New Firmware</button>
        </div>
        <p id='status'></p>
        <div id='progress-bar'><div id='progress'></div></div>
        
       <div class="back-button-group">

       <button class="go-back-button" onclick="location.href='/api'">Dashboard</button>
        <button class="go-back-button" onclick="location.href='/'">Clear</button>
        </div>

    </div>
    
    <h4><strong>mazur888 -- 2025</strong></h4>

    <script>
        var dropArea = document.getElementById('drop-area');
        var fileInput = document.getElementById('fileInput');
        var uploadButton = document.getElementById('uploadButton');
        var statusText = document.getElementById('status');
        var progressBar = document.getElementById('progress');

        uploadButton.disabled = true;

        dropArea.addEventListener('click', function() { fileInput.click(); });
        fileInput.addEventListener('change', function() { handleFiles(this.files); });

        dropArea.addEventListener('dragover', function(e) {
            e.preventDefault();
            dropArea.classList.add('highlight');
            statusText.innerText = 'Drop file to upload';
        });
        
        dropArea.addEventListener('dragleave', function() {
            dropArea.classList.remove('highlight');
            statusText.innerText = ''; // Clear status if no file is dragged
            if (fileInput.files.length === 0) {
                statusText.innerText = 'No file selected.';
                uploadButton.disabled = true;
            } else {
                statusText.innerText = `File selected: ${fileInput.files[0].name}`;
                uploadButton.disabled = false;
            }
        });
        
        dropArea.addEventListener('drop', function(e) {
            e.preventDefault();
            dropArea.classList.remove('highlight');
            handleFiles(e.dataTransfer.files);
        });
        
        function handleFiles(files) {
            if (files.length > 0) {
                fileInput.files = files;
                statusText.innerText = `File selected: ${files[0].name}`;
                uploadButton.disabled = false; 
            } else {
                statusText.innerText = 'No file selected.';
                uploadButton.disabled = true;
            }
            progressBar.style.width = '0%';
            progressBar.innerText = '';
        }
        
        uploadButton.addEventListener('click', startUpload);

        function startUpload() {
            if (!fileInput.files.length) {
                alert('Please select a file first.');
                return;
            }

            uploadButton.disabled = true;
            statusText.innerText = 'Starting upload...';
            progressBar.style.width = '0%';
            progressBar.innerText = '0%';

            var formData = new FormData();
            formData.append('update', fileInput.files[0]);
            var xhr = new XMLHttpRequest();
            xhr.open('POST', '/update', true);
            
            xhr.upload.onprogress = function(event) {
                if (event.lengthComputable) {
                    var percent = Math.round((event.loaded / event.total) * 100);
                    progressBar.style.width = percent + '%';
                    progressBar.innerText = percent + '%';
                    statusText.innerText = 'Upload progress: ' + percent + '%';
                }
            };
            
            xhr.onload = function() {
                if (xhr.status == 200) {
                    statusText.innerText = 'Upload completed successfully. Device rebooting...';
                    progressBar.style.width = '100%';
                    progressBar.innerText = '100%';
                } else {
                    statusText.innerText = 'Upload failed: ' + xhr.statusText;
                    progressBar.style.width = '0%'; 
                    progressBar.innerText = 'Error';
                    uploadButton.disabled = false;
                }
            };
            
            xhr.onerror = function() {
                statusText.innerText = 'Upload failed: Connection error.';
                progressBar.style.width = '0%'; 
                progressBar.innerText = 'Error';
                uploadButton.disabled = false; 
            };
            
            xhr.send(formData);
        }
    </script>
</body>
</html>

)rawliteral";
  server.send(200, "text/html", html);
}