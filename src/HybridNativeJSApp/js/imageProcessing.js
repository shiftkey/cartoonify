(function () {
    "use strict";

    var fileReference;

    function updateProgress(percent) {
        var newprogress = document.getElementById('progressbar');
        newprogress.value = percent;

        var progresstext = document.getElementById('progresstext');
        progresstext.textContent = percent + "%";
    }

    function makeProgressVisible(toggle) {
        var tag = document.getElementById("progress");
        // if (toggle)
            // hide
            // else
        //show
    }

    function displayImageFromPath(imgPath, imgTag) {
        Windows.Storage.StorageFile.getFileFromPathAsync(imgPath).then(
            function (imgFile) {
                var url = URL.createObjectURL(imgFile);
                var tag = document.getElementById(imgTag);
                tag.src = url;
            });
    }
  
    function cartoonifyImage() {
        updateProgress(0);
        var tmpFolder = Windows.Storage.ApplicationData.current.temporaryFolder;

        var dstImgPath = tmpFolder.path + "\\fred.jpg";

        // the magic happens right here
        var nativeObject = new imaging.WinRTComponent();

        nativeObject.transformImageAsync(imageProcessing.fileReference.path, dstImgPath).then(
            function () {
                    updateProgress(100);
                    makeProgressVisible(false);
                    displayImageFromPath(dstImgPath, "cartoonifiedPhoto");
                },
                function (error) {
                    makeProgressVisible(true);
                    updateProgress(0);
                },
                function (percent) {
                    updateProgress(percent);
                }
            );
    }

    function dataRequested(e) {
        var request = e.request;

        // Title is required
        var dataPackageTitle = "Cartoonified";
        if ((typeof dataPackageTitle === "string") && (dataPackageTitle !== "")) {
            if (imageProcessing.fileReference) {
                request.data.properties.title = dataPackageTitle;

                var streamReference = Windows.Storage.Streams.RandomAccessStreamReference.createFromFile(imageProcessing.fileReference);
                request.data.properties.thumbnail = streamReference;
                request.data.setStorageItems([imageProcessing.fileReference]);
                request.data.setBitmap(streamReference);
            } else {
                request.failWithDisplayText("Select an image you would like to share and try again.");
            }
        } else {
            request.failWithDisplayText("Geborken");
        }
    }


    WinJS.Namespace.define("imageProcessing", {
        fileReference : fileReference,
        cartoonifyImage: cartoonifyImage,
        displayImageFromPath: displayImageFromPath,
        dataRequested: dataRequested
    });

})();