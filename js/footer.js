/* This file is part of Nokia HEIF library
 *
 * Copyright (c) 2015-2026 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its subsidiaries. All rights are reserved.
 *
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia.
 */

function Footer () {
    this.inject = function () {
        document.body.insertAdjacentHTML('beforeend', `
            <a name="download"></a>
            <div class="heif-download-container row">
                <div class="heif-download-content col-xs-12 col-sm-6">
                    <div class="heif-download-content-title">
                    Download
                    </div>
                    <div class="heif-download-content-link">
                        <a href="https://github.com/nokiatech/heif">
                            ↓ HEIF Reader/Writer Source Code at GitHub
                        </a>
                    </div>
                    <div class="heif-download-content-link">
                        <a href="https://github.com/nokiatech/heif/tree/miaf">
                            ↓ MIAF Source Code GitHub
                        </a>
                    </div>
                </div>
                <div class="heif-download-content col-xs-12 col-sm-6">
                    <div class="heif-download-content-title">
                    External Links
                    </div>
                    <ul>
                        <li><a href="http://standards.iso.org/ittf/PubliclyAvailableStandards/c066067_ISO_IEC_23008-12_2017.zip">HEIF standard ISO/IEC 23008-12:2017</a></li>
                    </ul>
                </div>
            </div>
            <div class="heif-footer-container">
                <small><span>Copyright \u00A9 Nokia Technologies 2015-2026</span></small>
                <small><span>Contact: <a href="mailto:heif@nokia.com">heif@nokia.com</a> | <a href="https://github.com/nokiatech/heif/issues">GitHub</a></span></small>
            </div>
        `);
    };

    this.inject();
}
