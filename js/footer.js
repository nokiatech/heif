// Copyright (c) 2015-2017, Nokia Technologies Ltd.
// All rights reserved.
//
// Licensed under the Nokia High-Efficiency Image File Format (HEIF)
// License (the "License").
//
// You may not use the High-Efficiency Image File Format except in
// compliance with the License.
// The License accompanies the software and can be found in the file
// "LICENSE.TXT".
//
// You may also obtain the License at:
// https://nokiatech.github.io/heif/license.txt
//

function Footer () {
    this.inject = function () {
        document.body.innerHTML +=
        '   <a name="download"></a> \
            <div class="heif-download-container row"> \
                <div class="heif-download-content  col-xs-12 col-sm-6"> \
                    <div class="heif-download-content-title"> \
                    Download \
                    </div> \
                    <div class="heif-download-content-link"> \
                        <a href="http://phenix.int-evry.fr/jct/doc_end_user/current_document.php?id=10265"> \
                            <span class="glyphicon glyphicon-download"></span> HEIF Whitepaper \
                        </a> \
                    </div> \
                    <div class="heif-download-content-link"> \
                        <a href="https://github.com/nokiatech/heif"> \
                            <span class="glyphicon glyphicon-download"></span> HEIF Reader/Writer Source Code at GitHub\
                        </a> \
                    </div> \
                    <div class="heif-download-content-link"> \
                        <a href="https://github.com/nokiatech/heif/tree/gh-pages/js"> \
                            <span class="glyphicon glyphicon-download"></span> HEIF Reader JavaScript Implementation at GitHub\
                        </a> \
                    </div> \
                </div> \
                <div class="heif-download-content col-xs-12 col-sm-6"> \
                    <div class="heif-download-content-title"> \
                    External Links \
                    </div> \
                    <ul> \
                        <li><a href="http://mpeg.chiariglione.org/">MPEG</a></li> \
                        <li><a href="http://mpeg.chiariglione.org/standards/mpeg-4/iso-base-media-file-format">ISO Base Media File Format Specification</a></li> \
                    </ul> \
                </div> \
            </div> \
            \
            <div class="heif-footer-container"> \
                <small><span>Copyright &#169; Nokia Technologies 2015-2017</span></small> \
                <small><span>Contact: <a href="mailto:heif@nokia.com">heif@nokia.com</a></span></small>\
            </div> \
            \
            <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script> \
            <script src="js/bootstrap.min.js"></script> \
        ';
    };

    this.inject();
}
