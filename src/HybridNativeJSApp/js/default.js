// For an introduction to the Blank template, see the following documentation:
// http://go.microsoft.com/fwlink/?LinkId=232509
(function () {
    "use strict";

    var app = WinJS.Application;

    app.onactivated = function (eventObject) {
        if (eventObject.detail.kind === Windows.ApplicationModel.Activation.ActivationKind.launch) {
            if (eventObject.detail.previousExecutionState !== Windows.ApplicationModel.Activation.ApplicationExecutionState.terminated) {
                // TODO: This application has been newly launched. Initialize
                // your application here.
            } else {
                // TODO: This application has been reactivated from suspension.
                // Restore application state here.
            }
            WinJS.UI.processAll().then(function () {
                var dataTransferManager = Windows.ApplicationModel.DataTransfer.DataTransferManager.getForCurrentView();
                dataTransferManager.addEventListener("datarequested", imageProcessing.dataRequested);
                document.getElementById("takePhoto").addEventListener("click", capture.photo, false);
                document.getElementById("cartoonify").addEventListener("click", imageProcessing.cartoonifyImage, false);
            });
        }
    };

    app.oncheckpoint = function (eventObject) {
    };

    app.start();
})();