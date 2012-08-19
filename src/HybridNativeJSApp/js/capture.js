(function () {
    "use strict";


    function capturePhoto() {
        var dialog = new Windows.Media.Capture.CameraCaptureUI();
        var aspectRatio = { width: 16, height: 9 };
        dialog.photoSettings.croppedAspectRatio = aspectRatio;
        dialog.captureFileAsync(Windows.Media.Capture.CameraCaptureUIMode.photo).done(
            function (file) {
                if (file) {
                    var blobUrl = URL.createObjectURL(file);
                    imageProcessing.fileReference = file;
                    document.getElementById("capturedPhoto").src = blobUrl;
                }
            });
    }


    WinJS.Namespace.define("capture", {
        photo: capturePhoto
    });

})()