/**
 * Add gobals here
 */
var seconds 	= null;
var fileUploadTImerValue =  null;

/**
 * Initialize functions here.
 */
$(document).ready(function(){
	getUploadStatus();
});   

/**
 * Gets file name and size for display on the web page.
 */        
function getFileInfo() 
{
    var x = document.getElementById("selected_file");
    var file = x.files[0];

    document.getElementById("file_info").innerHTML = "<h4>File: " + file.name + "<br>" + "Size: " + file.size + " bytes</h4>";
}

/**
 * Handles the file upload.
 */
function uploadFile() 
{
    // Form Data
    var formData = new FormData();
    var fileSelect = document.getElementById("selected_file");
    
    if (fileSelect.files && fileSelect.files.length == 1) 
	{
        var file = fileSelect.files[0];
        formData.set("file", file, file.name);
        document.getElementById("upload_status").innerHTML = "Uploading " + file.name + " in Progress...";

        // Http Request
        var request = new XMLHttpRequest();

        request.upload.addEventListener("progress", updateProgress);
        request.open('POST', "/fileUpload");
        request.responseType = "blob";
        request.send(formData);
    } 
	else 
	{
        window.alert('Select A File First')
    }
}

/**
 * Progress on transfers from the server to the client (downloads).
 */
function updateProgress(oEvent) 
{
    if (oEvent.lengthComputable) 
	{
        getUploadStatus();
    } 
	else 
	{
        window.alert('total size is unknown')
    }
}

/**
 * Posts the file upload status.
 */
function getUploadStatus() 
{
    var xhr = new XMLHttpRequest();
    var requestURL = "/fileUploadStatus";
    xhr.open('POST', requestURL, false);
    xhr.send('upload_status');

    if (xhr.readyState == 4 && xhr.status == 200) 
	{		
        var response = JSON.parse(xhr.responseText);
						
	 	document.getElementById("Date & Time").innerHTML = response.compile_date + " - " + response.compile_time

        if (response.upload_status == 1) 
		{
    		// Set the countdown timer time
            seconds = 10;
            // Start the countdown timer
            fileUploadRebootTimer();
        } 
        else if (response.upload_status == -1)
		{
            document.getElementById("upload_status").innerHTML = "!!! Upload Error !!!";
        }
    }
}

/**
 * Displays the reboot countdown.
 */
function fileUploadRebootTimer() 
{	
    document.getElementById("upload_status").innerHTML = "File Upload Complete !" + seconds;

    if (--seconds == 0) 
	{
        clearTimeout(fileUploadTImerValue);
        window.location.reload();
    } 
	else 
	{
        fileUploadTImerValue = setTimeout(fileUploadRebootTimer, 3000);
    }
}


