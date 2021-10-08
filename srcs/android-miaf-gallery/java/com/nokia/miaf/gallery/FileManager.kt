/*
 * This file is part of Nokia HEIF applications
 *
 * Copyright (c) 2019-2021 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be disclosed to others without the prior written consent of Nokia.
 *
 */

package com.nokia.miaf.gallery

import java.io.File
val POSTFIXES: Array<String> = arrayOf(".heic", ".heif", ".hif", ".avcs", ".avci", ".heifs", ".heics")

fun loadFolder(folder: File): List<File>
{
    var fileList: MutableList<File> = mutableListOf()
    if (folder.exists() && folder.isDirectory)
    {
        var files = folder.listFiles()
        for (file in files)
        {
            if (!file.isDirectory)
            {
                for (postfix in POSTFIXES)
                {
                    if ( file.name.endsWith(postfix, true))
                    {
                        fileList.add(file)
                    }
                }
            }
        }
    }
    return fileList
}
