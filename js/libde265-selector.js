// Copyright (c) 2015, Nokia Technologies Ltd.
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

function isIE(userAgent) {
  userAgent = userAgent || navigator.userAgent;
  return userAgent.indexOf("MSIE ") > -1 || userAgent.indexOf("Trident/") > -1;
}

if (isIE()) {
    console.log("Running on Internet Explorer");
    document.write('<script src="js/libde265-ie.js"></script>');
} else {
    document.write('<script src="js/libde265.min.js"></script>');
}

