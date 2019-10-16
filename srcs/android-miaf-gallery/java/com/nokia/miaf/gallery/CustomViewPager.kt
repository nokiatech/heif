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

import android.content.Context
import android.support.v4.view.ViewPager
import android.util.AttributeSet
import android.view.MotionEvent

class CustomViewPager : ViewPager {
    private var mEnableScrolling = true

    constructor(context: Context) : super(context)

    constructor(context: Context, attrs: AttributeSet) : super(context, attrs)

    override fun onInterceptTouchEvent(ev: MotionEvent): Boolean {
        return if (mEnableScrolling) {
            super.onInterceptTouchEvent(ev)
        } else {
            false
        }
    }

    override fun onTouchEvent(ev: MotionEvent): Boolean {
        return if (mEnableScrolling) {
            super.onTouchEvent(ev)
        } else {
            false
        }
    }

    fun enableScrolling(enabled: Boolean) {
        mEnableScrolling = enabled
    }
}
