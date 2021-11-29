mergeInto(LibraryManager.library, {
    measureCanvas: undefined,
    measureCanvasCtx: undefined,
    _jsMeasureChar__deps: ['measureCanvas', 'measureCanvasCtx'],
    _jsMeasureChar: function (codePoint, fontSize, bold, outCharWidth, outCharMidBaseline, outCharDescentFromBaseline) {
        if (!_measureCanvas) {
            _measureCanvas = document.createElement('canvas');
            _measureCtx = _measureCanvas.getContext('2d');
            _measureCtx.textBaseline = 'middle';
        }
        _measureCtx.font = (bold ? 'bold ' : '') + fontSize + 'px/1 system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Ubuntu, "Helvetica Neue", sans-serif';
        let text = String.fromCodePoint(codePoint);
        let textMetrics = _measureCtx.measureText(text);
        HEAPF32[outCharWidth >> 2] = textMetrics.width;
        HEAPF32[outCharMidBaseline >> 2] = textMetrics.actualBoundingBoxAscent;
        HEAPF32[outCharDescentFromBaseline >> 2] = textMetrics.actualBoundingBoxDescent;
    },
    atlasCanvas: undefined,
    atlasCanvasCtx: undefined,
    _jsAtlasCanvasInit__deps: ['atlasCanvas', 'atlasCanvasCtx'],
    _jsAtlasCanvasInit: function (width, height) {
        _atlasCanvas = document.createElement('canvas');
        _atlasCanvas.width = width;
        _atlasCanvas.height = height;
        _atlasCanvasCtx = _atlasCanvas.getContext('2d');
        _atlasCanvasCtx.fillStyle = 'black';
        _atlasCanvasCtx['globalCompositeOperator'] = 'copy';
        _atlasCanvasCtx.globalAlpha = 0;
        _atlasCanvasCtx.fillRect(0, 0, canvas.width, canvas.height);
        _atlasCanvasCtx.globalAlpha = 1;
    },
    _jsAtlasCanvasPutChar__deps: ['atlasCanvasCtx'],
    _jsAtlasCanvasPutChar: function (codePoint, fontSize, bold, r, g, b, x, middleBaselineY) {
        let text = String.fromCodePoint(codePoint);
        _atlasCanvasCtx.fillStyle = `rgb(${r} ${g} ${b})`;
        _atlasCanvasCtx.font = (bold ? 'bold ' : '') + fontSize + 'px/1 system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Ubuntu, "Helvetica Neue", sans-serif';
        _atlasCanvasCtx.textBaseline = 'middle';
        _atlasCanvasCtx.fillText(text, x, middleBaselineY);
    },
    uploadFlipped: function (img, isWebGl2) {
        GLctx.pixelStorei(0x9240/*GLctx.UNPACK_FLIP_Y_WEBGL*/, true);
        GLctx.texImage2D(0xDE1/*GLctx.TEXTURE_2D*/, 0, 0x1908/*GLctx.RGBA*/, 0x1908/*GLctx.RGBA*/, 0x1401/*GLctx.UNSIGNED_BYTE*/, img);
        if (isWebGl2) {
            GLctx.generateMipmap(0xDE1/*GLctx.TEXTURE_2D*/);
        }
        GLctx.pixelStorei(0x9240/*GLctx.UNPACK_FLIP_Y_WEBGL*/, false);
    },
    _jsAtlasCanvasUploadToTexture__deps: ['atlasCanvas', 'atlasCanvasCtx', 'uploadFlipped'],
    _jsAtlasCanvasUploadToTexture: function (isWebGl2) {
        if (false) // Enable for debugging
        {
            document.body.appendChild(_atlasCanvas);
            document.getElementById("canvas").style.display = "none";
            document.body.style.backgroundColor = "white";
            debugger;
        }
        _uploadFlipped(_atlasCanvas, isWebGl2);
        _atlasCanvas = undefined;
        _atlasCanvasCtx = undefined;
    },
    _jsLoadTextureFromUrl__deps: ['uploadFlipped'],
    _jsLoadTextureFromUrl: function (
        glTexture, cUrl, isWebGl2, loadedCallbackInt, callbackEpochInt, userDataInt, outW, outH
    ) {
        let img = new Image();
        img.onload = function () {
            HEAPU32[outW >> 2] = img.width;
            HEAPU32[outH >> 2] = img.height;
            GLctx.bindTexture(0xDE1/*GLctx.TEXTURE_2D*/, GL.textures[glTexture]);
            _uploadFlipped(img, isWebGl2);
            Module.ccall(
                'imageLoadedCallback',
                null,
                ['number', 'number', 'number', 'number', 'number'],
                [loadedCallbackInt, callbackEpochInt, userDataInt, img.width, img.height]
            );
        };
        img.onerror = function (event, source, lineno, colno, error) {
            // debugger;
        }
        img.crossOrigin = "";
        let url = UTF8ToString(cUrl);
        const isCdnEnabled = false; // Set by the build script
        const cdnBaseUrl = "";
        if (isCdnEnabled && !url.startsWith("http")) {
            url = `${cdnBaseUrl}/${url}`
        }
        img.src = url;
    },
    _jsSetCursor: function (type) {
        let typeStr = ['default', 'grabbing'][type];
        if (typeStr === undefined) {
            console.error('cursor type is undefined:', type);
        }
        document.getElementById("canvas").style.cursor = typeStr;
    },
    _jsSetFocus: function () {
        document.getElementById("canvas").focus();
    },
    _jsGetLocationHash: function (maxLength, outHash) {
        const hashStr = window.location.hash.substring(1);
        const charsToCopy = Math.min(hashStr.length, maxLength - 1);
        for (var i = 0; i < charsToCopy; i++) {
            HEAPU8[outHash] = hashStr[i].charCodeAt(0);
            outHash++;
        }
        HEAPU8[outHash] = 0;
    },
    _jsSetLocationHash: function (hash) {
        window.location.hash = UTF8ToString(hash);
    },
    _jsSetLocationHashChangeCallback: function (cCallbackName, userDataInt) {
        const callbackName = UTF8ToString(cCallbackName);
        window.onhashchange = function () {
            let hashStr = window.location.hash.substring(1);
            Module.ccall(callbackName, null, ["string", "number"], [hashStr, userDataInt]);
        }
    },
    _jsOpen: function (url) {
        window.open(UTF8ToString(url));
    },
    _jsGetScrollLineHeight: function () {
        const iframe = document.createElement('iframe');
        iframe.src = '#';
        document.body.appendChild(iframe);
        const iwin = iframe.contentWindow;
        const idoc = iwin.document;
        idoc.open();
        idoc.write('<!DOCTYPE html><html><head></head><body><span>a</span></body></html>');
        idoc.close();
        const span = idoc.body.firstElementChild;
        const lineHeight = span.offsetHeight;
        document.body.removeChild(iframe);
        return lineHeight;
    },
    _jsAlert: function (cMessage) {
        const message = UTF8ToString(cMessage);
        alert(message);
    },
    _jsWebGlFailedFallback: function () {
        document.getElementById("canvas").remove();
        document.body.style = null; // Undo emscripten fullscreen magic

        const fallbackDiv = document.createElement("div");
        fallbackDiv.innerText = "Looks like this browser doesn't support WebGL, which is sad since the app fully depends on it."
        fallbackDiv.appendChild(document.createElement("br"));
        const hashStr = window.location.hash.substring(1);
        const linkDiv = document.createElement("div");
        if (hashStr) {
            linkDiv.appendChild(document.createTextNode("You can view the thread you were linked to at "));
            const url = `https://twitter.com/visakanv/status/${hashStr}`;
            const link = document.createElement("a");
            link.href = url;
            link.innerText = url;
            linkDiv.appendChild(link);
        } else {
            linkDiv.appendChild(document.createTextNode("You can view Visa's threads at "));
            const url = `https://twitter.com/visakanv`;
            const link = document.createElement("a");
            link.href = url;
            link.innerText = url;
            linkDiv.appendChild(link);
        }
        fallbackDiv.appendChild(linkDiv);
        document.body.appendChild(fallbackDiv);
    }
});
