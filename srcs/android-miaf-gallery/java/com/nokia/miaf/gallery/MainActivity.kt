/*
 * This file is part of Nokia HEIF applications
 *
 * Copyright (c) 2019 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: heif@nokia.com
 *
 * This software, including documentation, is protected by copyright controlled by Nokia Corporation. All rights are reserved.
 * Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior written consent of Nokia Corporation.
 * This material also contains confidential information which may not be disclosed to others without the prior written consent of Nokia.
 *
 */

package com.nokia.miaf.gallery

import android.Manifest
import android.app.Activity
import android.app.AlertDialog
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Bundle

class MainActivity : Activity() {

    private val PERMISSION_REQUEST_READ_EXTERNAL = 0

    private var requestPending = false

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }

    override fun onResume() {
        super.onResume()
        if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED)
        {
            launchGridActivity()
        }
        else if (!requestPending)
        {
            requestRequiredPermissions()
        }
    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == PERMISSION_REQUEST_READ_EXTERNAL)
        {
            if (grantResults.size == 2 && grantResults.get(0) == PackageManager.PERMISSION_GRANTED && grantResults.get(1) == PackageManager.PERMISSION_GRANTED)
            {
                launchGridActivity()
            }
        }
    }

    private fun launchGridActivity()
    {
        val intent = Intent(this, MiafViewerActivity::class.java)
        intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP or Intent.FLAG_ACTIVITY_NEW_TASK)
        startActivity(intent)
        finish()
    }

    private fun requestRequiredPermissions()
    {
        if (shouldShowRequestPermissionRationale(Manifest.permission.READ_EXTERNAL_STORAGE))
        {
            val dialogBuilder = AlertDialog.Builder(this)
            dialogBuilder.setTitle(R.string.permissions_title)
            dialogBuilder.setMessage(R.string.permissions_message)
            dialogBuilder.setPositiveButton(R.string.accept, { _, _ ->
                requestPermissions(arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE), PERMISSION_REQUEST_READ_EXTERNAL)
                requestPending = true
            })

            dialogBuilder.setNegativeButton(R.string.decline, {_, _ ->
                finish()
            })
            val dialog = dialogBuilder.create()
            dialog.show()
        }
        else
        {
            requestPermissions(arrayOf(Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE), PERMISSION_REQUEST_READ_EXTERNAL)
            requestPending = true
        }
    }
}
