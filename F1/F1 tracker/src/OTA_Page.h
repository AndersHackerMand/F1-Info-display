#pragma once
#include <WebServer.h>
#include <Preferences.h>

extern WebServer    server;
extern Preferences  preferences;

void handleOTAUpdatePage() {
      Serial.println("HTTP GET /  OTA page");

  String html = R"rawliteral(

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=0.8">
    <title>OTA Update</title>
 <style>
  :root {
    --f1-red:       #e10600;
    --asphalt:      #121212;
    --rubber:       #1a1a1a;
    --tread:        #2e2e2e;
    --text-light:   #f1f1f1;
    --text-muted:   #999999;
    --accent-yellow:#ffd700;
    --border-radius:6px;
  }

  * {
    box-sizing: border-box;
    margin: 0; padding: 0;
  }

  body {
    font-family: 'Orbitron', sans-serif;
    background: var(--asphalt);
    color: var(--text-light);
    text-align: center;
    padding: 2rem 1rem;
  }

  header h1 {
    font-size: 2.5rem;
    letter-spacing: 4px;
    margin-bottom: 2rem;
    background:
      linear-gradient(45deg, var(--accent-yellow) 25%, transparent 25%),
      linear-gradient(-45deg, var(--accent-yellow) 25%, transparent 25%),
      linear-gradient(45deg, transparent 75%, var(--accent-yellow) 75%),
      linear-gradient(-45deg, transparent 75%, var(--accent-yellow) 75%);
    background-size: 20px 20px;
    background-position: 0 0, 0 10px, 10px -10px, -10px 0px;
    -webkit-background-clip: text;
    color: transparent;
  }

  .container {
    max-width: 600px;
    margin: auto;
    background: var(--rubber);
    padding: 2rem;
    border-radius: var(--border-radius);
    box-shadow: 0 4px 12px rgba(0,0,0,0.7);
  }

  #drop-area {
    background: var(--tread);
    border: 3px dashed var(--text-muted);
    padding: 3rem 1rem;
    border-radius: var(--border-radius);
    transition: background 0.3s, border-color 0.3s;
  }
  #drop-area.highlight {
    border-color: var(--accent-yellow);
    background: #333;
  }
  #drop-area p {
    font-size: 1.2rem;
    color: var(--text-muted);
    margin-top: 1rem;
    font-weight: bold;
    letter-spacing: 1px;
  }
  #drop-area svg {
    width: 60px;
    height: 60px;
    fill: var(--text-muted);
  }
  #drop-area.highlight svg {
    fill: var(--accent-yellow);
  }

  .button-group {
    margin-top: 2rem;
    display: flex;
    gap: 1rem;
    justify-content: center;
  }
  button {
    background: var(--f1-red);
    color: #fff;
    border: none;
    padding: 0.75rem 2rem;
    font-size: 1rem;
    font-weight: bold;
    border-radius: var(--border-radius);
    cursor: pointer;
    transition: transform 0.2s, box-shadow 0.2s;
    position: relative;
  }
  button:hover {
    transform: translateY(-3px);
    box-shadow: 0 6px 12px rgba(0,0,0,0.5);
  }
  button:disabled {
    background: #555;
    cursor: not-allowed;
  }

  /* Progress bar */
  #progress-bar {
    margin: 2rem 0 1rem;
    background: #333;
    border-radius: var(--border-radius);
    overflow: hidden;
    box-shadow: inset 0 2px 6px rgba(0,0,0,0.7);
  }
  #progress {
    height: 1.5rem;
    width: 0%;
    background: var(--accent-yellow);
    transition: width 0.3s ease;
  }

  #status {
    color: var(--text-muted);
    font-style: italic;
    margin-top: 1rem;
  }

  h4 {
    margin-top: 2rem;
    font-size: 1.1rem;
    color: var(--text-light);
  }

  hr {
    border: none;
    height: 1px;
    margin: 2rem 0;
    background: linear-gradient(to right,
      transparent, var(--text-muted), transparent);
  }
</style>

</head>
<body>
    <h1>F1 Tracker OTA</h1>
    <h4><strong>mazur888 -- 2025</strong></h4>

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