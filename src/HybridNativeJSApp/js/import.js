(function () {
    "use strict";


    function importPhoto() {
        var openPicker = new Windows.Storage.Pickers.FileOpenPicker();
        openPicker.viewMode = Windows.Storage.Pickers.PickerViewMode.thumbnail;
        openPicker.suggestedStartLocation = Windows.Storage.Pickers.PickerLocationId.picturesLibrary;
        openPicker.fileTypeFilter.replaceAll([".png", ".jpg", ".jpeg"]);

        openPicker.pickSingleFileAsync().then(function (file) {
            if (file) {
                var blobUrl = URL.createObjectURL(file);
                document.getElementById("capturedPhoto").src = blobUrl;
                var tmpFolder = Windows.Storage.ApplicationData.current.temporaryFolder;
                var newFile = "input" + file.fileType;
                file.copyAsync(tmpFolder, newFile, Windows.Storage.NameCollisionOption.replaceExisting)
                    .done(function(newFile) {
                        imageProcessing.fileReference = newFile;
                    });
            } else {
                console.log("Operation cancelled.");
            }
        });
    }

    WinJS.Namespace.define("select", {
        fromFile: importPhoto
    });

})()