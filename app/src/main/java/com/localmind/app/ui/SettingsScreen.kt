package com.localmind.app.ui

import android.net.Uri
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import com.localmind.app.LlmEngine
import java.io.File
import java.io.FileOutputStream

@Composable
fun SettingsScreen() {
    val context = LocalContext.current
    var statusText by remember { mutableStateOf("لم يتم تحميل أي موديل") }
    var isLoading by remember { mutableStateOf(false) }

    val filePickerLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.GetContent()
    ) { uri: Uri? ->
        if (uri != null) {
            isLoading = true
            statusText = "جاري نسخ الموديل وتحميله... قد يستغرق دقيقة"
            try {
                // نسخ الملف للمجلد الداخلي للتطبيق
                val inputStream = context.contentResolver.openInputStream(uri)
                val modelFile = File(context.filesDir, "model.gguf")
                val outputStream = FileOutputStream(modelFile)
                inputStream?.copyTo(outputStream)
                inputStream?.close()
                outputStream.close()

                // تحميل الموديل في C++
                val success = LlmEngine.loadModel(modelFile.absolutePath)
                if (success) {
                    statusText = "✅ تم تحميل الموديل بنجاح! جاهز للدردشة."
                } else {
                    statusText = "❌ فشل تحميل الموديل."
                }
            } catch (e: Exception) {
                statusText = "❌ خطأ: ${e.message}"
            } finally {
                isLoading = false
            }
        }
    }

    Column(modifier = Modifier.fillMaxSize().padding(16.dp), horizontalAlignment = Alignment.CenterHorizontally) {
        Text("الإعدادات وإدارة الموديل", style = MaterialTheme.typography.headlineSmall)
        Spacer(modifier = Modifier.height(24.dp))
        
        Text(text = statusText, style = MaterialTheme.typography.bodyLarge)
        
        Spacer(modifier = Modifier.height(24.dp))
        
        Button(
            onClick = { filePickerLauncher.launch("*/*") },
            enabled = !isLoading
        ) {
            Text(if (isLoading) "جاري التحميل..." else "اختر ملف الموديل (GGUF)")
        }

        Spacer(modifier = Modifier.height(32.dp))
        Text("معلومات النظام من C++:", style = MaterialTheme.typography.titleMedium)
        Text(text = LlmEngine.getHelloFromJni(), style = MaterialTheme.typography.bodySmall)
    }
}
