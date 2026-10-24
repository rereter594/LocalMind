package com.localmind.app

object LlmEngine {
    init {
        System.loadLibrary("localmind")
    }

    external fun getHelloFromJni(): String
    external fun loadModel(path: String): Boolean
    external fun generate(prompt: String): String
}
