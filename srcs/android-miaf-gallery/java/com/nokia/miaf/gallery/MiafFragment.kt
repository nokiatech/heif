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

import android.os.Bundle
import android.support.v4.app.Fragment
import android.view.*

import com.nokia.heif.*
import com.nokia.heif.utility.miaf.MIAFReader
import kotlinx.android.synthetic.main.fragment_miaf.*
import java.io.File
import java.util.HashSet

class MiafFragment : Fragment()
{
    private val mMIAF : MIAFReader = MIAFReader()
    private var mLoaded = false
    private var mFilename: String? = null
    private var mCurrentDisplayed: Base? = null

    private var mIsVisible = false

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        return  inflater.inflate(R.layout.fragment_miaf, container, false) as ViewGroup
    }

    override fun onResume() {
        super.onResume()
        if (!mLoaded)
        {
            loadImage()
        }
    }

    override fun setUserVisibleHint(isVisibleToUser: Boolean) {
        super.setUserVisibleHint(isVisibleToUser)
        mIsVisible = isVisibleToUser
    }

    private fun updateDisplayImages()
    {
        val constraints = MIAFReader.Constraints()
        val roles = HashSet<MIAFReader.OutputRole>()
        roles.add(MIAFReader.OutputRole.MASTER)

        val content = mMIAF.getContent(constraints, roles)
        miaf_view.setContent(content.master)
        mCurrentDisplayed = content.master
    }

    fun loadHEIFImage(file: File)
    {
        mFilename = file.absolutePath
        mMIAF.load(mFilename)
    }

    private fun loadImage()
    {
        updateDisplayImages()
        registerForContextMenu(miaf_view)
        miaf_view.setOnClickListener{ togglePlayback() }
        miaf_view.setOnLongClickListener {
            miaf_view.showContextMenu()
            return@setOnLongClickListener true
        }
    }

    private fun togglePlayback()
    {
        if (mCurrentDisplayed is VideoTrack)
        {
            miaf_view.togglePlayback()
        }
    }
}
