package com.localmind.app.ui

import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@Composable
fun SettingsScreen() {
    Column(modifier = Modifier.fillMaxSize().padding(16.dp)) {
        Text("الإعدادات", style = MaterialTheme.typography.headlineSmall)
        Spacer(modifier = Modifier.height(16.dp))
        Text("الموديل الحالي: Qwen2.5-1.5B")
        Spacer(modifier = Modifier.height(24.dp))
        Text("درجة الإبداع (Temperature)")
        Slider(value = 0.7f, onValueChange = {})
    }
}
