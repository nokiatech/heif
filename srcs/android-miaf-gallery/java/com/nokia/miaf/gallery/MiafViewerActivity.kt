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
import android.os.Environment
import android.support.v4.app.Fragment
import android.support.v4.app.FragmentActivity
import android.support.v4.app.FragmentManager
import android.support.v4.app.FragmentStatePagerAdapter
import android.support.v4.view.PagerAdapter
import android.support.v4.view.ViewPager
import kotlinx.android.synthetic.main.activity_image_viewer.*
import java.io.File
import java.lang.ref.WeakReference

class MiafViewerActivity : FragmentActivity(), ViewPager.OnPageChangeListener
{
    inner class ImagePageAdapter(fm: FragmentManager, fileList: List<File>) : FragmentStatePagerAdapter(fm)
    {
        private val mFiles = fileList


        override fun getItem(position: Int): Fragment
        {
            val imageFragment = MiafFragment()
            imageFragment.loadHEIFImage(mFiles[position])
            if (position >= mViews.size)
            {
                mViews.add(WeakReference(imageFragment))
            }
            else
            {
                mViews.set(position, WeakReference(imageFragment))
            }

            return imageFragment
        }

        override fun getCount(): Int
        {
            return mFiles.size
        }
    }

    private var mPagerAdapter: PagerAdapter? = null

    private val mViews: ArrayList<WeakReference<MiafFragment>> = ArrayList()

    private val IMAGE_FOLDER_PATH = Environment.getExternalStorageDirectory().absolutePath + "/miaf-files/"

    override fun onPageScrollStateChanged(state: Int)
    {
    }

    override fun onPageScrolled(position: Int, positionOffset: Float, positionOffsetPixels: Int)
    {
    }

    override fun onPageSelected(position: Int)
    {
    }

    override fun onCreate(savedInstanceState: Bundle?)
    {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_image_viewer)
        view_pager.addOnPageChangeListener(this)
    }

    override fun onResume()
    {
        super.onResume()
        val folder = File(IMAGE_FOLDER_PATH)
        mPagerAdapter = ImagePageAdapter(supportFragmentManager, loadFolder(folder))
        view_pager.adapter = mPagerAdapter
    }
}
