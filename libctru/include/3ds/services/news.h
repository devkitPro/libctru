/**
 * @file news.h
 * @brief NEWS (Notification) service.
 */
#pragma once

/// Notification header data.
typedef struct {
	bool dataSet;
	bool unread;
	bool enableJPEG;
	u8 unkFlag1;
	u8 unkFlag2;
	u64 processID;
	u8 unkData[24];
	u64 time;
	u16 title[32];
} NotificationHeader;

/// Initializes NEWS.
Result newsInit(void);

/// Exits NEWS.
void newsExit(void);

/**
 * @brief Adds a notification to the home menu Notifications applet.
 * @param title UTF-16 title of the notification.
 * @param titleLength Number of characters in the title, not including the null-terminator.
 * @param message UTF-16 message of the notification, or NULL for no message.
 * @param messageLength Number of characters in the message, not including the null-terminator.
 * @param image Data of the image to show in the notification, or NULL for no image.
 * @param imageSize Size of the image data in bytes.
 * @param jpeg Whether the image is a JPEG or not.
 */
Result NEWS_AddNotification(const u16* title, u32 titleLength, const u16* message, u32 messageLength, const void* imageData, u32 imageSize, bool jpeg);

/**
 * @brief Gets current total notifications number.
 * @param num Pointer where total number will be saved.
 */
Result NEWS_GetTotalNotifications(u32* num);

/**
 * @brief Sets a custom header for a specific notification.
 * @param news_id Identification number of the notification.
 * @param header Notification header to set.
 */
Result NEWS_SetNotificationHeader(u32 news_id, NotificationHeader header);

/**
 * @brief Gets the header of a specific notification.
 * @param news_id Identification number of the notification.
 * @param header Pointer where header of the notification will be saved.
 */
Result NEWS_GetNotificationHeader(u32 news_id, NotificationHeader* header);

/**
 * @brief Gets the message of a specific notification.
 * @param news_id Identification number of the notification.
 * @param message Pointer where UTF-16 message of the notification will be saved.
 */
Result NEWS_GetNotificationMessage(u32 news_id, u16* message);

/**
 * @brief Gets the image of a specific notification.
 * @param news_id Identification number of the notification.
 * @param buffer Pointer where MPO image of the notification will be saved.
 * @param size Pointer where size of the image data will be saved in bytes.
 */
Result NEWS_GetNotificationImage(u32 id, u8* buffer, u32* size);
