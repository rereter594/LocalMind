package com.localmind.app

object LlmEngine {
    init {
        System.loadLibrary("localmind")
    }

    external fun getHelloFromJni(): String
}
