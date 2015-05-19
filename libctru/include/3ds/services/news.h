#pragma once

/*
	Requires access to "news:u" service.
*/


Result newsInit();
Result newsExit();

/* NEWSU_AddNotification()
About: Adds a notification to the home menu Notifications applet.

  title	UTF-16 title of the notification.
  titleLength	Number of characters in the title, not including the null-terminator.
  title	UTF-16 message of the notification, or NULL for no message.
  titleLength	Number of characters in the message, not including the null-terminator.
  image	Data of the image to show in the notification, or NULL for no image.
  imageSize	Size of the image data in bytes.
  jpeg		Whether the image is a JPEG or not.
*/
Result NEWSU_AddNotification(const u16* title, u32 titleLength, const u16* message, u32 messageLength, const void* imageData, u32 imageSize, bool jpeg);
