package com.localmind.app.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Send
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.localmind.app.LlmEngine
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

data class ChatMessage(val text: String, val isUser: Boolean)

@Composable
fun ChatScreen() {
    var messages by remember { 
        mutableStateOf(listOf(ChatMessage("مرحباً! أنا LocalMind. جاهز للدردشة معك.", false))) 
    }
    var inputText by remember { mutableStateOf("") }
    var isLoading by remember { mutableStateOf(false) }
    val scope = rememberCoroutineScope()

    Column(modifier = Modifier.fillMaxSize()) {
        LazyColumn(
            modifier = Modifier.weight(1f).fillMaxWidth().padding(horizontal = 16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp),
            contentPadding = PaddingValues(vertical = 16.dp)
        ) {
            items(messages) { message -> ChatBubble(message) }
        }

        if (isLoading) {
            LinearProgressIndicator(modifier = Modifier.fillMaxWidth())
        }

        Surface(
            shadowElevation = 8.dp,
            color = MaterialTheme.colorScheme.surface,
            modifier = Modifier.fillMaxWidth()
        ) {
            Row(
                modifier = Modifier.padding(12.dp).fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically
            ) {
                OutlinedTextField(
                    value = inputText,
                    onValueChange = { inputText = it },
                    modifier = Modifier.weight(1f),
                    placeholder = { Text("اكتب رسالتك هنا...") },
                    shape = RoundedCornerShape(24.dp),
                    enabled = !isLoading
                )
                Spacer(modifier = Modifier.width(8.dp))
                IconButton(
                    onClick = {
                        if (inputText.isNotBlank() && !isLoading) {
                            val userMsg = inputText
                            messages = messages + ChatMessage(userMsg, true)
                            inputText = ""
                            isLoading = true
                            
                            // تجهيز "البرومبت" الكامل مع تاريخ المحادثة
                            val promptBuilder = StringBuilder()
                            // تعليمات النظام (عشان يرد بالعربي ويركز)
                            promptBuilder.append("<|im_start|>system\nأنت مساعد ذكي ومفيد اسمك LocalMind. أجب بالعربية بوضوح وإيجاز.<|im_end|>\n")
                            
                            // إضافة كل الرسائل السابقة
                            for (msg in messages) {
                                if (msg.isUser) {
                                    promptBuilder.append("<|im_start|>user\n${msg.text}<|im_end|>\n")
                                } else {
                                    promptBuilder.append("<|im_start|>assistant\n${msg.text}<|im_end|>\n")
                                }
                            }
                            // فتح دور المساعد عشان يبدأ يكتب
                            promptBuilder.append("<|im_start|>assistant\n")

                            scope.launch {
                                val reply = withContext(Dispatchers.IO) {
                                    LlmEngine.generate(promptBuilder.toString())
                                }
                                val finalReply = if (reply.isBlank()) "عذراً، لم أفهم. هل يمكنك إعادة الصياغة؟" else reply
                                messages = messages + ChatMessage(finalReply, false)
                                isLoading = false
                            }
                        }
                    },
                    modifier = Modifier.background(
                        if (isLoading) MaterialTheme.colorScheme.surfaceVariant else MaterialTheme.colorScheme.primary, 
                        RoundedCornerShape(50)
                    )
                ) {
                    Icon(Icons.Default.Send, contentDescription = "Send", tint = MaterialTheme.colorScheme.onPrimary)
                }
            }
        }
    }
}

@Composable
fun ChatBubble(message: ChatMessage) {
    val alignment = if (message.isUser) Alignment.CenterEnd else Alignment.CenterStart
    val color = if (message.isUser) MaterialTheme.colorScheme.primary else MaterialTheme.colorScheme.secondaryContainer
    val textColor = if (message.isUser) MaterialTheme.colorScheme.onPrimary else MaterialTheme.colorScheme.onSecondaryContainer
    val shape = if (message.isUser) RoundedCornerShape(16.dp, 16.dp, 0.dp, 16.dp) else RoundedCornerShape(16.dp, 16.dp, 16.dp, 0.dp)

    Box(modifier = Modifier.fillMaxWidth(), contentAlignment = alignment) {
        Surface(color = color, shape = shape, modifier = Modifier.widthIn(max = 300.dp)) {
            Text(text = message.text, modifier = Modifier.padding(12.dp), color = textColor)
        }
    }
}
