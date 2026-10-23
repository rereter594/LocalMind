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
        
        Text("الموديل الحالي:", style = MaterialTheme.typography.titleMedium)
        Text("Qwen2.5-1.5B-Instruct Q4", color = MaterialTheme.colorScheme.primary)
        
        Spacer(modifier = Modifier.height(24.dp))
        
        Text("درجة الإبداع (Temperature): 0.7", style = MaterialTheme.typography.titleMedium)
        Slider(value = 0.7f, onValueChange = {})
        
        Spacer(modifier = Modifier.height(24.dp))
        
        Text("حجم السياق (Context): 4096", style = MaterialTheme.typography.titleMedium)
    }
}
