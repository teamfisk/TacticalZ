/*
 * Copyright (c) 2013 Adobe Systems Incorporated. All rights reserved.
 *  
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *  
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *  
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 * 
 */

(function () {
    "use strict";
    var start = 0;
    var zmq = require('zmq');
    var publisher = zmq.socket('pub');

    publisher.bind('tcp://*:5555', function (err) {
        if (err) {
            console.log(err);
        }
        else {
            console.log("5555");
        }
    });



    process.on('SIGINT', function () {
        publisher.close();
    });


    var PLUGIN_ID = require("./package.json").name,
          MENU_ID = "TeamFisk plugins",
          MENU_LABEL = "$$$/JavaScripts/Generator/TeamFisk plugins/Menu=Live Texturing RAMDisk";

    var _generator = null;

    // Initialize script here 
    function init(generator, config) {
        _generator = generator;
        _generator.addMenuItem(MENU_ID, MENU_LABEL, true, false)
           .then(
               function () {
                   console.log("Menu created", MENU_ID);
               }, function () {
                   console.error("Menu creation failed", MENU_ID);
               }
           );

        _generator.onPhotoshopEvent("imageChanged", handleImageChanged);
    }

    var lastSent = 0;
    function handleImageChanged(document) {

        var start = new Date().getTime();
        if (start - lastSent > 200) {
            _generator.getDocumentInfo(document.id).then(
            function (document) {
                // console.log(new Date().getTime() / 1000);
                // console.log(stringify(document.timeStamp));

                //lastSent = document.timeStamp
                // console.log(stringify(document));
                //console.log("Received complete document:", stringify(document));
                //var str = 'var options = new PNGSaveOptions(); app.activeDocument.saveAs (new File("D:/HejHej.png"),options, false);';
                var str = 'app.activeDocument.save()';
                _generator.evaluateJSXString(str).done();

                var size = (4 + document.file.length + 1);
                var message = new Buffer(size);

                var offset = 0;

                //console.log("document.file.length: " + document.file.length);
                //console.log("document.fileName: " + document.file);

                message.writeInt32LE(document.file.length + 1, offset);

                offset += 4;

                var fileName = document.file.replace(/\\/g, "/");
                message.write(fileName, offset, fileName.length, 'utf8');
                offset += fileName.length;
                message.writeUInt8(0, offset); //Null byte
                offset += 1;

                publisher.send(message);

                var asd = new Date().getTime();
                var time = asd - start;
                console.log(time);
                lastSent = new Date().getTime();
                console.log("LastSent: " + lastSent);
            });
        }
    }

    function stringify(object) {
        try {
            return JSON.stringify(object, null, "    ");
        } catch (e) {
            console.error(e);
        }
        return String(object);
    }

    //Declare entery function in the script
    exports.init = init;
}());