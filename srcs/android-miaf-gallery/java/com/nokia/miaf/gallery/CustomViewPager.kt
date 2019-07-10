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
