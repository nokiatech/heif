/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

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
                        <a href="https://github.com/nokiatech/heif/tree/miaf"> \
                            <span class="glyphicon glyphicon-download"></span> MIAF Source Code GitHub\
                        </a> \
                    </div> \
                </div> \
                <div class="heif-download-content col-xs-12 col-sm-6"> \
                    <div class="heif-download-content-title"> \
                    External Links \
                    </div> \
                    <ul> \
                        <li><a href="http://mpeg.chiariglione.org/">MPEG</a></li> \
                        <li><a href="http://standards.iso.org/ittf/PubliclyAvailableStandards/c066067_ISO_IEC_23008-12_2017.zip">HEIF standard ISO/IEC 23008-12:2017</a></li> \
                        <li><a href="https://www.iso.org/standard/74417.html">MIAF standard ISO/IEC 23000-22:2019</a></li> \
                        <li><a href="http://mpeg.chiariglione.org/standards/mpeg-4/iso-base-media-file-format">ISO Base Media File Format Specification</a></li> \
                    </ul> \
                </div> \
            </div> \
            \
            <div class="heif-footer-container"> \
                <small><span>Copyright &#169; 2015-2021 Nokia Corporation and/or its subsidiary(-ies)</span></small> \
                <small><span>Contact: <a href="mailto:heif@nokia.com">heif@nokia.com</a></span></small>\
            </div> \
            \
            <script src="https://ajax.googleapis.com/ajax/libs/jquery/1.11.3/jquery.min.js"></script> \
            <script src="js/bootstrap.min.js"></script> \
        ';
    };

    this.inject();
}
