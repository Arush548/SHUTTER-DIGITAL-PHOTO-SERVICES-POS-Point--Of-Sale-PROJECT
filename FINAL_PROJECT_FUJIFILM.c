//========== CPEP-112 1st Semester Programming Project
//========== Adviser: Liduvina Lagon Namocatcat
//========== SHUTER DIGITAL : PHOTO SERVICES POS
//========== By: F4

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>
#include <curl/curl.h>
#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")


#define MAX_CART_ITEMS 100
#define EMAIL_USERNAME "yourgmail@gmail.com"
#define EMAIL_PASSWORD "your_app_password"

typedef struct {
    char description[100];
    double price;
    int quantity;
} CartItem;

CartItem cart[MAX_CART_ITEMS];
int cartItemCount = 0;
double cartTotal = 0.0;
int hasTarpaulin = 0;

// ==================== VOIDS ===========================
void Header(void);
void setGreenText(void);
void PrintCentered(const char *text);
void PrintCenteredF(const char *format, ...);
void PrintCenteredPrompt(const char *prompt);
int getValidInteger(const char *prompt, int min, int max);
double getValidDouble(const char *prompt, double min);
void inputPassword(char *password, int size);
void getValidString(const char *prompt, char *buffer, int size);
void getValidNumber(const char *prompt, char *buffer, int size);
static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp);

void AddToCart(const char *description, double price, int quantity);
void ViewCart(void);
void EditCart(void);
void Checkout(void);
void SaveReceiptToFile(const char *customerName,
                       const char *phone,
                       const char *email,
                       const char *cashier,
                       double total,
                       double cash);
void SaveTarpOrderToFile(const char *barcode, const char *name, const char *phone, double amount, const char *services);
static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp)
{
    const char **payload_text = (const char **)userp;

    if (*payload_text) {
        size_t len = strlen(*payload_text);
        memcpy(ptr, *payload_text, len);
        *payload_text = NULL;
        return len;
    }
    return 0;
}
//==============MENU VOIDS===================
void Lamination(void);
void RushID(void);
void Visa(void);
void OtherSizes(void);
void PhotoPrinting(void);
void TarpulinPrinting(void);
void Scanning(void);
void SearchTransaction(void);
void NewOrder(void);

void ManageCashiers(void);
void AddNewCashier(void);
void ViewAllCashiers(void);
void RemoveCashier(void);
void ChangeCashierPassword(void);

void AdminMenu(void);
void CashierMenu(void);

//========== LOGIN VOIDS ==============
void AdminLogin(void);
void CashierLogin(void);
void Login(void);

void generateUniqueText(char *text, int size);
void generateCode39(char *barcode, const char *text);
void PrintTarpaulinDirect(const char *name, const char *phone, const char *service,
                          double total, const char *code39);
void ClaimTarpOrder(void);


int SendEmailAuto(const char *to, const char *subject, const char *body) {
    CURL *curl;
    CURLcode res;

    // Sender credentials
    const char *from = "agrcatada@universityofbohol.edu.ph";
    const char *user = "agrcatada@universityofbohol.edu.ph";
    const char *app_password = "mxtk ghfk kxck gtnw";  // Your Gmail App Password

    curl = curl_easy_init();
    if (!curl)
        return 0;

    struct curl_slist *recipients = NULL;

    curl_easy_setopt(curl, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, app_password);

    curl_easy_setopt(curl, CURLOPT_URL, "smtp://smtp.gmail.com:587");
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, from);

    recipients = curl_slist_append(recipients, to);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    char payload[3000];
    snprintf(payload, sizeof(payload),
             "To: %s\r\n"
             "From: %s\r\n"
             "Subject: %s\r\n"
             "\r\n"
             "%s\r\n",
             to, from, subject, body);

    const char *payload_ptr = payload;
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
    curl_easy_setopt(curl, CURLOPT_READDATA, &payload_ptr);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    res = curl_easy_perform(curl);

    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

char loggedInUser[50] = "";


// ==================== UTILITIES ====================

void setGreenText(void) {
    system("color 0A");
}

int GetConsoleWidth(void) {
    #ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
            return csbi.srWindow.Right - csbi.srWindow.Left + 1;
        }
    #endif
    return 80;
}

void PrintLogoBMPDirectToSerial(HANDLE hSerial)
{
    // Load the BMP image
    HBITMAP hBitmap = (HBITMAP)LoadImage(NULL, "logo.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (!hBitmap) {
        MessageBox(NULL, "Failed to load logo BMP.", "Error", MB_OK);
        return;
    }

    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    // Convert BMP to monochrome bitmap data for thermal printer
    int widthBytes = ((bmp.bmWidth + 7) / 8);
    unsigned char *bitmapData = (unsigned char *)malloc(widthBytes * bmp.bmHeight);
    if (!bitmapData) {
        DeleteObject(hBitmap);
        return;
    }

    // Create memory DC to read pixels
    HDC hMemDC = CreateCompatibleDC(NULL);
    HBITMAP hOld = (HBITMAP)SelectObject(hMemDC, hBitmap);

    for (int y = 0; y < bmp.bmHeight; y++) {
        for (int x = 0; x < bmp.bmWidth; x++) {
            COLORREF color = GetPixel(hMemDC, x, y);
            int byteIndex = y * widthBytes + x / 8;
            int bitIndex  = 7 - (x % 8);
            if (GetRValue(color) < 128 && GetGValue(color) < 128 && GetBValue(color) < 128)
                bitmapData[byteIndex] |= (1 << bitIndex);
            else
                bitmapData[byteIndex] &= ~(1 << bitIndex);
        }
    }

    SelectObject(hMemDC, hOld);
    DeleteDC(hMemDC);

    DWORD bytesWritten;

    // --- Send ESC/POS command for image ---
    unsigned char header[8] = {0x1D, 0x76, 0x30, 0x00, (unsigned char)(widthBytes % 256), (unsigned char)(widthBytes / 256), (unsigned char)(bmp.bmHeight % 256), (unsigned char)(bmp.bmHeight / 256)};
    WriteFile(hSerial, header, sizeof(header), &bytesWritten, NULL);
    WriteFile(hSerial, bitmapData, widthBytes * bmp.bmHeight, &bytesWritten, NULL);

    free(bitmapData);
    DeleteObject(hBitmap);
}


void SaveReceiptToFile(const char *customerName,
                       const char *phone,
                       const char *email,
                       const char *cashier,
                       double total,
                       double cash)
{
    system("mkdir C:\\Receipts >nul 2>&1");

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char filename[260];
    snprintf(filename, sizeof(filename),
        "C:\\Receipts\\receipt_%04d-%02d-%02d_%02d%02d%02d.txt",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec);

    FILE *fp = fopen(filename, "w");
    if (!fp) return;

    fprintf(fp, "SHUTER DIGITAL PHOTO SERVICES\n");
    fprintf(fp, "Ground Flr., Tubigon\n");
    fprintf(fp, "Bohol\n\n");

    fprintf(fp, "Customer: %s\nPhone: %s\nEmail: %s\n", customerName, phone, email);
    fprintf(fp, "Served by: %s\n", loggedInUser);
    fprintf(fp, "-----------------------------\n");
    fprintf(fp, "Service        Qty   Price    Total\n");
    fprintf(fp, "-----------------------------\n");

    for (int i = 0; i < cartItemCount; i++) {
        double lineTotal = cart[i].price * cart[i].quantity;
        fprintf(fp, "%-14s %3d  P%6.2f  P%7.2f\n",
                cart[i].description,
                cart[i].quantity,
                cart[i].price,
                lineTotal);
    }

    fprintf(fp, "-----------------------------\n");
    fprintf(fp, "Total:  P%.2f\nCash:   P%.2f\nChange: P%.2f\n",
            total, cash, cash - total);

    fclose(fp);
}

void PrintAndSaveReceipt(const char *customerName, const char *phone, const char *email,
    const char *cashier, double total, double cash, const char *barcode,
    int isTarpaulin
)
{
    HANDLE hSerial;
    DCB dcb = {0};
    DWORD bytesWritten;

    // Open COM port once
    hSerial = CreateFile("\\\\.\\COM4", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        PrintCentered("Error: Cannot open COM port. Check printer connection.");
        getch();
        return;
    }

    dcb.DCBlength = sizeof(dcb);
    GetCommState(hSerial, &dcb);
    dcb.BaudRate = 9600;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    SetCommState(hSerial, &dcb);

    unsigned char data[2048];
    int pos = 0;

    // --- Initialize printer ---
    data[pos++] = 0x1B; data[pos++] = 0x40;

    // --- Center alignment ---
    data[pos++] = 0x1B; data[pos++] = 0x61; data[pos++] = 0x01;

    // --- Logo ---
    PrintLogoBMPDirectToSerial(hSerial); // new function: prints logo directly to the same serial

    // --- Header ---
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    pos += sprintf((char*)&data[pos], "TIN#:763-247-907-00001\n");
    pos += sprintf((char*)&data[pos], "Ground flr., Tubigon\n");
    pos += sprintf((char*)&data[pos], "Comm'l Complex, Centro,\n");
    pos += sprintf((char*)&data[pos], "Tubigon, Bohol\n");
    pos += sprintf((char*)&data[pos], "Date: %02d-%02d-%04d\n", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
    pos += sprintf((char*)&data[pos], "Time: %02d-%02d-%02d\n", t->tm_hour, t->tm_min , t->tm_sec);

    pos += sprintf((char*)&data[pos], "----------------------------\n");

    // --- Customer Info ---
    pos += sprintf((char*)&data[pos], "Customer: %s\nPhone: %s\nEmail: %s\n", customerName, phone, email);
    pos += sprintf((char*)&data[pos], "Served By: %s\n", loggedInUser);
    pos += sprintf((char*)&data[pos], "----------------------------\n");
    pos += sprintf((char*)&data[pos], "Service   Qty  Price    Total\n");
    pos += sprintf((char*)&data[pos], "-----------------------------\n");

    for (int i = 0; i < cartItemCount; i++) {
        double lineTotal = cart[i].quantity * cart[i].price;

    pos += sprintf((char*)&data[pos], "%-10s%3dpcs P%6.2f P%7.2f\n",
        cart[i].description,
        cart[i].quantity,
        cart[i].price,
        lineTotal
    );
}

    pos += sprintf((char*)&data[pos], "-----------------------------\n");
    pos += sprintf((char*)&data[pos], "Total:  P%.2f\nCash:   P%.2f\nChange: P%.2f\n",
    total, cash, cash - total
);

    // --- Barcode (if tarpaulin) ---
    if (isTarpaulin && barcode) {
        data[pos++] = 0x1D; data[pos++] = 0x48; data[pos++] = 0x02;
        data[pos++] = 0x1D; data[pos++] = 0x68; data[pos++] = 80;
        data[pos++] = 0x1D; data[pos++] = 0x6B; data[pos++] = 0x04;
        pos += sprintf((char*)&data[pos], "%s", barcode);
        data[pos++] = 0x00; // end barcode
    }

    pos += sprintf((char*)&data[pos], "\nThank you!Come Again!\n====================\n\n");

    // --- Send all at once ---
    WriteFile(hSerial, data, pos, &bytesWritten, NULL);
    CloseHandle(hSerial);

    // --- Save text file as before ---
   SaveReceiptToFile(customerName, phone, email, cashier, total, cash);


    PrintCentered("Receipt sent directly to printer.");
    getch();
}



void AutoEmailReceipt(const char *customerEmail,
                      const char *service,
                      double total,
                      double cash)
{
    if (customerEmail == NULL || strlen(customerEmail) == 0)
        return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char subject[200];
    snprintf(subject, sizeof(subject),
             "Official Receipt - Shutter Digital (P%.2f)", total);

    char body[2000];
    snprintf(body, sizeof(body),

             "===================================\n"
             "  SHUTTER DIGITAL PHOTO SERVICES\n"
             "  TIN#-763-247-907-00001\n"
             "=====================================\n\n"
              "           E-RECEIPT\n"
             " Date: %02d-%02d-%04d\n"
             " Time: %02d:%02d:%02d\n"
             " Service:   %s\n"
             " Served by: %s\n"
             "-------------------------------------\n"
             " Total:     P%.2f\n"
             " Cash:      P%.2f\n"
             " Change:    P%.2f\n"
             "-------------------------------------\n\n"
             " Thank you for choosing Shutter Digital!\n\n"
             " Address:\n"
             " Ground Floor, Tubigon Commercial Complex\n"
             " Centro 6329, Tubigon, Bohol\n\n"
             " Contact:\n"
             " 09398567968 / 09494145607\n\n"
             "This is an automated receipt.\n"
             "\n\n"
             "Embrace every moment, \nfor each one is a PRESENT.\n",
             t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
             t->tm_hour, t->tm_min, t->tm_sec,
             service,
             loggedInUser,
             total,
             cash,
             cash - total);

    SendEmailAuto(customerEmail, subject, body);
}

void PrintCentered(const char *text) {
    int width = GetConsoleWidth();

    char buffer[1024];
    strncpy(buffer, text, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char *line = strtok(buffer, "\n");
    while (line != NULL) {
        int len = strlen(line);
        int padding = (width - len) / 2;

        if (padding > 0) {
            printf("%*s", padding, "");
        }
        printf("%s\n", line);

        line = strtok(NULL, "\n");
    }
}

void PrintCenteredF(const char *format, ...) {
    char buffer[512];
    va_list args;

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    PrintCentered(buffer);
}

void PrintCenteredPrompt(const char *prompt) {
    int width = GetConsoleWidth();
    int len = strlen(prompt);
    int padding = (width - len - 20) / 2;

    if (padding > 0) {
        printf("%*s", padding, "");
    }
    printf("%s", prompt);
}

void Header(void) {
    system("cls");
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int hour12 = t->tm_hour % 12;
    if (hour12 == 0) hour12 = 12;
    char *ampm = (t->tm_hour >= 12) ? "PM" : "AM";

    printf("\n\n");

    PrintCentered("+============================================+");
    PrintCentered("SHUTTER DIGITAL PRINTING SERVICES");

    char dateTime[100];
    snprintf(dateTime, sizeof(dateTime),
             "Date: %02d-%02d-%04d  Time: %02d:%02d:%02d %s",
             t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
             hour12, t->tm_min, t->tm_sec, ampm);
    PrintCentered(dateTime);
    PrintCentered("+============================================+");
    printf(" Logged in as: %s", loggedInUser);
    printf("\n");
}

int getValidInteger(const char *prompt, int min, int max) {
    char input[100];
    int value;

    while (1) {
        PrintCenteredPrompt(prompt);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            PrintCentered("Error reading input. Try again.");
            continue;
        }

        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0) {
            PrintCentered("Input cannot be empty. Try again.");
            continue;
        }

        int valid = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if (i == 0 && input[i] == '-') continue;
            if (!isdigit((unsigned char)input[i])) {
                valid = 0;
                break;
            }
        }

        if (!valid) {
            PrintCentered("Invalid input! Please enter a number.");
            continue;
        }

        value = atoi(input);

        if (value < min || value > max) {
            char msg[100];
            snprintf(msg, sizeof(msg), "Please enter a number between %d and %d.", min, max);
            PrintCentered(msg);
            continue;
        }

        return value;
    }
}

void getValidNumber(const char *prompt, char *buffer, int size) {
    int valid;

    do {
        valid = 1;
        PrintCenteredPrompt(prompt);
        fgets(buffer, size, stdin);

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strlen(buffer) == 0) {
            valid = 0;
        }

        for (int i = 0; buffer[i] != '\0'; i++) {
            if (!isdigit((unsigned char)buffer[i])) {
                valid = 0;
                break;
            }
        }

        if (!valid) {
            PrintCentered("Invalid input. Numbers only.");
        }

    } while (!valid);
}

double getValidDouble(const char *prompt, double min) {
    char input[100];
    double value;
    char *endptr;

    while (1) {
        PrintCenteredPrompt(prompt);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            PrintCentered("Error reading input. Try again.");
            continue;
        }

        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0) {
            PrintCentered("Input cannot be empty. Try again.");
            continue;
        }

        value = strtod(input, &endptr);

        if (*endptr != '\0' && *endptr != '\n') {
            PrintCentered("Invalid input! Please enter a valid number.");
            continue;
        }

        if (value < min) {
            char msg[100];
            snprintf(msg, sizeof(msg), "Value must be at least %.2f", min);
            PrintCentered(msg);
            continue;
        }

        return value;
    }
}

void getValidString(const char *prompt, char *buffer, int size) {
    while (1) {
        PrintCenteredPrompt(prompt);

        if (fgets(buffer, size, stdin) == NULL) {
            PrintCentered("Error reading input. Try again.");
            continue;
        }

        buffer[strcspn(buffer, "\n")] = 0;

        if (strlen(buffer) == 0) {
            PrintCentered("Input cannot be empty. Try again.");
            continue;
        }

        return;
    }
}

void inputPassword(char *password, int size) {
    int i = 0;
    char ch;
    while (1) {
        ch = getch();
        if (ch == 13) {
            password[i] = '\0';
            printf("\n");
            break;
        } else if (ch == 8) {
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else if (i < size - 1) {
            password[i++] = ch;
            printf("*");
        }
    }
}

void SaveTarpOrderToFile(const char *barcode, const char *name, const char *phone, double amount, const char *services) {
    FILE *fp = fopen("tarp_orders.txt", "a");
    if (!fp) {
        PrintCentered("ERROR: Cannot save tarpaulin order to file.");
        return;
    }
    fprintf(fp, "%s|%s|%s|%.2f|%s|PENDING\n", barcode, name, phone, amount, services);
    fclose(fp);
}

// ==================== CART ====================

void AddToCart(const char *description, double price, int quantity) {
    if (cartItemCount >= MAX_CART_ITEMS) {
        Header();
        PrintCentered("Cart is full! Please checkout first.");
        getch();
        return;
    }

    if (strstr(description, "Tarpulin") != NULL) {
        hasTarpaulin = 1;
    }

    int found = -1;
    for (int i = 0; i < cartItemCount; i++) {
        if (strcmp(cart[i].description, description) == 0) {
            found = i;
            break;
        }
    }

    if (found != -1) {
        cart[found].quantity += quantity;
    } else {
        strncpy(cart[cartItemCount].description, description, 99);
        cart[cartItemCount].description[99] = '\0';
        cart[cartItemCount].price = price;
        cart[cartItemCount].quantity = quantity;
        cartItemCount++;
    }

    cartTotal += price * quantity;

    Header();
    char msg[200];
    snprintf(msg, sizeof(msg), "Added to cart: %s x%d = P%.2f", description, quantity, price * quantity);
    PrintCentered(msg);
    snprintf(msg, sizeof(msg), "Cart Total: P%.2f", cartTotal);
    PrintCentered(msg);
    printf("\n");
    PrintCentered("Press any key to continue...");
    getch();
}

void ViewCart(void) {
    Header();
    PrintCentered("==================");
    PrintCentered("  SHOPPING CART");
    PrintCentered("==================");
    printf("\n");

    if (cartItemCount == 0) {
        PrintCentered("Your cart is empty!");
        printf("\n");
    } else {
        char line[200];
        snprintf(line, sizeof(line), "%-40s %5s %10s", "ITEM", "QTY", "PRICE");
        PrintCentered(line);
        PrintCentered("--------------------------------------------------------");

        for (int i = 0; i < cartItemCount; i++) {
            double itemTotal = cart[i].price * cart[i].quantity;
            snprintf(line, sizeof(line), "%-40s x%-4d P%9.2f",
                   cart[i].description,
                   cart[i].quantity,
                   itemTotal);
            PrintCentered(line);
        }

        PrintCentered("--------------------------------------------------------");
        snprintf(line, sizeof(line), "%46s P%9.2f", "TOTAL:", cartTotal);
        PrintCentered(line);
        printf("\n");
    }

    PrintCentered("Press any key to continue...");
    getch();
}
void ClearCart(void)
{ cartItemCount = 0;
 cartTotal = 0.0;
  hasTarpaulin = 0;
}
void RecomputeCart(void)
{
    cartTotal = 0.0;
    hasTarpaulin = 0;

    for (int i = 0; i < cartItemCount; i++) {
        cartTotal += cart[i].price * cart[i].quantity;

        if (strstr(cart[i].description, "Tarpulin") != NULL) {
            hasTarpaulin = 1;
        }
    }
}


void EditCart(void)
{
    int choice, index, newQty;
    char confirm;

    if (cartItemCount == 0) {
        Header();
        PrintCentered("Your cart is empty!");
        getch();
        return;
    }

    while (1) {
        system("cls");
        Header();
        PrintCentered("EDIT CART");
        PrintCentered("============================");
        printf("\n");

        for (int i = 0; i < cartItemCount; i++) {
            char line[200];
            snprintf(line, sizeof(line), "%d. %-35s x%-3d P%8.2f",
                     i + 1,
                     cart[i].description,
                     cart[i].quantity,
                     cart[i].price * cart[i].quantity);
            PrintCentered(line);
        }

        printf("\n");
        PrintCentered("1. Add / Edit Quantity");
        PrintCentered("2. Remove Item");
        PrintCentered("3. Clear Cart");
        PrintCentered("0. Back");
        printf("\nChoice: ");
        scanf("%d", &choice);

        if (choice == 0)
            break;

        /* ===== ADD / EDIT QUANTITY ===== */
        else if (choice == 1) {
            printf("Enter item number: ");
            scanf("%d", &index);

            if (index < 1 || index > cartItemCount) {
                PrintCentered("Invalid item number.");
                getch();
                continue;
            }

            index--;

            printf("Current quantity: %d\n", cart[index].quantity);
            printf("Enter new quantity: ");
            scanf("%d", &newQty);

            if (newQty <= 0) {
                PrintCentered("Quantity must be greater than zero.");
                getch();
                continue;
            }

            printf("Confirm update quantity? (Y/N): ");
            scanf(" %c", &confirm);

            if (confirm == 'Y' || confirm == 'y') {
                cart[index].quantity = newQty;
                RecomputeCart();
                PrintCentered("Quantity updated successfully.");
            } else {
                PrintCentered("Action cancelled.");
            }

            getch();
        }

        /* ===== REMOVE ITEM ===== */
        else if (choice == 2) {
            printf("Enter item number to remove: ");
            scanf("%d", &index);

            if (index < 1 || index > cartItemCount) {
                PrintCentered("Invalid item number.");
                getch();
                continue;
            }

            index--;

            printf("Remove \"%s\"? (Y/N): ", cart[index].description);
            scanf(" %c", &confirm);

            if (confirm == 'Y' || confirm == 'y') {
                for (int i = index; i < cartItemCount - 1; i++) {
                    cart[i] = cart[i + 1];
                }
                cartItemCount--;
                RecomputeCart();
                PrintCentered("Item removed.");
            } else {
                PrintCentered("Action cancelled.");
            }

            getch();
        }

        /* ===== CLEAR CART ===== */
        else if (choice == 3) {
            printf("Are you sure you want to clear the cart? (Y/N): ");
            scanf(" %c", &confirm);

            if (confirm == 'Y' || confirm == 'y') {
                ClearCart();
                PrintCentered("Cart Edited Successfully.");
                getch();
                break;
            } else {
                PrintCentered("Action cancelled.");
                getch();
            }
        }
    }
}


// ==================== CHECKOUT ====================

void PrintReceiptDirect(const char *customerName, const char *phone, const char *email,
                        const char *cashier, const char *services, double total, double cash,
                        const char *barcode, int isTarpaulin)
{
    HANDLE hSerial;
    DCB dcb = {0};
    DWORD bytesWritten;

    // Open COM4 once
    hSerial = CreateFile("\\\\.\\COM4", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (hSerial == INVALID_HANDLE_VALUE) {
        PrintCentered("Error: Cannot open COM port.");
        getch();
        return;
    }

    dcb.DCBlength = sizeof(dcb);
    GetCommState(hSerial, &dcb);
    dcb.BaudRate = 9600;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    SetCommState(hSerial, &dcb);

    unsigned char data[4096];
    int pos = 0;

    // ESC @
    data[pos++] = 0x1B; data[pos++] = 0x40;

    // Center alignment
    data[pos++] = 0x1B; data[pos++] = 0x61; data[pos++] = 0x01;

    // --- Logo ---
    PrintLogoBMPDirectToSerial(hSerial);

    // Header text

   time_t now = time(NULL);
    struct tm *t = localtime(&now);

    pos += sprintf((char*)&data[pos], "TIN#:763-247-907-00001\n");
    pos += sprintf((char*)&data[pos], "Ground flr., Tubigon\n");
    pos += sprintf((char*)&data[pos], "Comm'l Complex, Centro,\n");
    pos += sprintf((char*)&data[pos], "Tubigon, Bohol\n");
    pos += sprintf((char*)&data[pos], "Date: %02d-%02d-%04d\n", t->tm_mday, t->tm_mon + 1, t->tm_year + 1900);
    pos += sprintf((char*)&data[pos], "Time: %02d-%02d-%02d\n", t->tm_hour, t->tm_min , t->tm_sec);

    pos += sprintf((char*)&data[pos], "----------------------------\n");

    // --- Customer Info ---
    pos += sprintf((char*)&data[pos], "Customer: %s\nPhone: %s\nEmail: %s\n", customerName, phone, email);
    pos += sprintf((char*)&data[pos], "----------------------------\n");

    // --- Services & Total ---
    pos += sprintf((char*)&data[pos], "%s\n", services);
    pos += sprintf((char*)&data[pos], "Total: P%.2f\nCash: P%.2f\nChange: P%.2f\n", total, cash, cash - total);
    // Barcode if tarpaulin
    if (isTarpaulin && barcode) {
        data[pos++] = 0x1D; data[pos++] = 0x48; data[pos++] = 0x02; // HRI below
        data[pos++] = 0x1D; data[pos++] = 0x68; data[pos++] = 80;  // height
        data[pos++] = 0x1D; data[pos++] = 0x6B; data[pos++] = 0x04; // Code39
        pos += sprintf((char*)&data[pos], "%s", barcode);
        data[pos++] = 0x00;
    }
    pos += sprintf((char*)&data[pos], "----------------------------\n");
    pos += sprintf((char*)&data[pos], "For Inquiries:\n");
    pos += sprintf((char*)&data[pos], "Contact: 09494145607");
    pos += sprintf((char*)&data[pos], "\n");
    pos += sprintf((char*)&data[pos], "09398567968");
    pos += sprintf((char*)&data[pos], "\n");
    pos += sprintf((char*)&data[pos], "Email: irishlynn123@yahoo.com");
    pos += sprintf((char*)&data[pos], "\n");
    pos += sprintf((char*)&data[pos], "Facebook: Fujifilm Tubigon\n");
    pos += sprintf((char*)&data[pos], "\nThank you! Come Again!\n====================\n");
    pos += sprintf((char*)&data[pos], "\n");

    WriteFile(hSerial, data, pos, &bytesWritten, NULL);
    CloseHandle(hSerial);

    PrintCentered("Receipt sent directly to printer.");
    getch();
}

// ===================== CHECKOUT =====================
void generateUniqueText(char *text, int size) {
    int counter = 1000;

    FILE *file = fopen("counter.txt", "r");
    if (file) {
        fscanf(file, "%d", &counter);
        fclose(file);
    }

    counter++;
    snprintf(text, size, "TARP-%04d", counter);

    file = fopen("counter.txt", "w");
    if (file) {
        fprintf(file, "%d", counter);
        fclose(file);
    }
}

#include <dirent.h>

typedef struct {
    char service[128];
    double total;
} ServiceSummary;
void SaveTransactionToCSV(
    const char *customerName,
    const char *customerPhone,
    const char *service,
    double price,
    int quantity,
    double cash,
    const char *status
) {
    // Ensure folder exists
    system("mkdir C:\\ReceiptsCSV >nul 2>&1");

    // Get current date
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char csvPath[256];
    snprintf(csvPath, sizeof(csvPath),
        "C:\\ReceiptsCSV\\transactions_%04d-%02d-%02d.csv",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday
    );

    FILE *fp = fopen(csvPath, "a");
    if (!fp) return;

    // Write header if file is new
    fseek(fp, 0, SEEK_END);
    if (ftell(fp) == 0) {
        fprintf(fp,
            "Date,Time,Customer,Phone,Service,Price,Qty,Total,Cash,Status\n"
        );
    }

    double total = price * quantity;

    fprintf(fp,
        "%04d-%02d-%02d,%02d:%02d:%02d,\"%s\",\"%s\",\"%s\",%.2f,%d,%.2f,%.2f,%s\n",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec,
        customerName,
        customerPhone,
        service,
        price,
        quantity,
        total,
        cash,
        status
    );

    fclose(fp);
}



void Checkout(void)
{
    system("cls");
    Header();

    if(cartTotal == 0){
            printf("\n");
            PrintCentered("Cart is Empty!");
            getch();
        return NewOrder();
    }


    char customerName[100] = "";
    char customerEmail[100] = "";
    char customerPhone[50] = "";
    char cashierName[100] = "DEFAULT CASHIER";

    getValidString("Customer Name: ", customerName, sizeof(customerName));
    getValidString("Customer Email (Gmail): ", customerEmail, sizeof(customerEmail));
    getValidNumber("Mobile Number: ", customerPhone, sizeof(customerPhone));

    double cash = 0.0;

    double regularTotal = 0.0, tarpTotal = 0.0;
    int hasRegular = 0, hasTarp = 0;
    char regularServices[512] = "";
    char tarpServices[512] = "";

   for (int i = 0; i < cartItemCount; i++) {
    SaveTransactionToCSV(
        customerName,
        customerPhone,
        cart[i].description,
        cart[i].price,
        cart[i].quantity,
        cash,
        "COMPLETED"
    );
}

    /* ===== COMPUTE TOTALS ===== */
    for (int i = 0; i < cartItemCount; i++) {
        double itemTotal = cart[i].price * cart[i].quantity;

        if (strstr(cart[i].description, "Tarpulin") != NULL) {
            tarpTotal += itemTotal;
            hasTarp = 1;
            if (strlen(tarpServices)) strcat(tarpServices, "; ");
            strcat(tarpServices, cart[i].description);
        } else {
            regularTotal += itemTotal;
            hasRegular = 1;
            if (strlen(regularServices)) strcat(regularServices, "; ");
            strcat(regularServices, cart[i].description);
        }
    }

    /* ===== REGULAR CUSTOMER DISCOUNT ===== */
    double discount = 0.0;
    if (hasRegular||hasTarp) {
        PrintCentered("Is this a Regular Customer?");
        PrintCentered("[1] Yes   [0] No");
        char d = getch();
        if (d == '1') {
            discount = regularTotal * 0.10;
            regularTotal -= discount;
        }
    }

    double grandTotal = regularTotal + tarpTotal;

    char msg[200];
    snprintf(msg, sizeof(msg), "Grand Total: P%.2f", grandTotal);
    PrintCentered(msg);

    cash = getValidDouble("Enter cash amount: P", grandTotal);

    /* ===== MERGE SERVICES ===== */
    char allServices[1024] = "";
    if (hasRegular) strcat(allServices, regularServices);
    if (hasTarp) {
        if (strlen(allServices)) strcat(allServices, "; ");
        strcat(allServices, tarpServices);
    }

    /* ===== TARPAULIN BARCODE ===== */
    char barcodeText[64] = "";
    if (hasTarp) {
        generateUniqueText(barcodeText, sizeof(barcodeText));
        SaveTarpOrderToFile(barcodeText, customerName, customerPhone, tarpTotal, tarpServices);
    }

    /* ===== PRINT RECEIPT ===== */
    PrintAndSaveReceipt(
    customerName,
    customerPhone,
    customerEmail,
    cashierName,
    grandTotal,
    cash,
    hasTarp ? barcodeText : NULL,
    hasTarp
);
// TRANSACTION SAVE TO CSV


    /* ===== AUTO EMAIL (FIXED) ===== */
    AutoEmailReceipt(
        customerEmail,
        allServices,
        grandTotal,
        cash
    );
    ClearCart();
}



void ClaimTarpOrder(void) {
    char code[128];
    char line[1024];

    Header();
    PrintCentered("===== CLAIM TARP ORDER =====");
    PrintCentered("\n");
    getValidString("Scan/Enter Barcode (e.g. TARP-0001): ", code, sizeof(code));

    FILE *in = fopen("tarp_orders.txt", "r");
    FILE *out = fopen("tarp_orders_tmp.txt", "w");
    if (!in || !out) {
        if (in) fclose(in);
        if (out) fclose(out);
        PrintCentered("Error: cannot open order file.");
        getch();
        return;
    }

    int found = 0;
    while (fgets(line, sizeof(line), in)) {
        char storedCode[128] = "";
        char customerName[256] = "";
        char customerPhone[64] = "";
        char amountStr[64] = "";
        char services[512] = "";
        char status[64] = "";

        char *p = strdup(line);
        if (!p) { fputs(line, out); continue; }

        p[strcspn(p, "\r\n")] = 0;

        char *tok = strtok(p, "|");
        if (tok) strncpy(storedCode, tok, sizeof(storedCode)-1);
        tok = strtok(NULL, "|");
        if (tok) strncpy(customerName, tok, sizeof(customerName)-1);
        tok = strtok(NULL, "|");
        if (tok) strncpy(customerPhone, tok, sizeof(customerPhone)-1);
        tok = strtok(NULL, "|");
        if (tok) strncpy(amountStr, tok, sizeof(amountStr)-1);
        tok = strtok(NULL, "|");
        if (tok) strncpy(services, tok, sizeof(services)-1);
        tok = strtok(NULL, "|");
        if (tok) strncpy(status, tok, sizeof(status)-1);

        if (strcmp(storedCode, code) == 0) {
            found = 1;
            fprintf(out, "%s|%s|%s|%s|%s|CLAIMED\n", storedCode, customerName, customerPhone, amountStr, services);
            Header();
            PrintCentered("Order found!");

            char msg[256];
            snprintf(msg, sizeof(msg), "Customer: %s", customerName);
            PrintCentered(msg);
            snprintf(msg, sizeof(msg), "Phone: %s", customerPhone);
            PrintCentered(msg);
            snprintf(msg, sizeof(msg), "Amount: P%s", amountStr);
            PrintCentered(msg);
            snprintf(msg, sizeof(msg), "Status: %s -> CLAIMED", status[0] ? status : "PENDING");
            PrintCentered(msg);
        }
        if (status == "CLAIMED"){
            char msg[256];
            snprintf(msg, sizeof(msg), "Status: %s -> ALREADY CLAIMED", status[0] ? status : "PENDING");
            PrintCentered(msg);
        }
            else {
            fputs(line, out);
        }

        free(p);
    }

    fclose(in);
    fclose(out);

    remove("tarp_orders.txt");
    rename("tarp_orders_tmp.txt", "tarp_orders.txt");

    if (found) {
        char msg[200];
        snprintf(msg, sizeof(msg), "Order %s has been marked as CLAIMED.", code);
        PrintCentered(msg);
    } else {
        PrintCentered("Order not found.");
    }
    if (line && code == 0){
}

    PrintCentered("Press any key...");
    getch();
}

// ==================== SERVICE MENUS ====================

void Lamination(void) {
    int choice;

    while (1) {
        Header();
        PrintCentered("=================");
        PrintCentered(" LAMINATION MENU");
        PrintCentered("=================");
        printf("\n");

        char msg[100];
        snprintf(msg, sizeof(msg), "CART TOTAL: P%.2f (%d items)", cartTotal, cartItemCount);
        PrintCentered(msg);
        printf("\n\n");

        PrintCentered("[1] SHORT BONDPAPER  P60");
        printf("\n");
        PrintCentered("[2] LONG BONDPAPER   P70");
        printf("\n");
        PrintCentered("[3] LEGAL BONDPAPER  P70");
        printf("\n");
        PrintCentered("[4] 65mm x 95mm      P20");
        printf("\n");
        PrintCentered("[5] 80mm x 110mm     P25");
        printf("\n");
        PrintCentered("[6] 95mm x 115mm     P30");
        printf("\n\n");
        PrintCentered("[0] BACK TO SERVICES");
        printf("\n");

        choice = getValidInteger("Choose: ", 0, 6);

        if (choice == 0) return;

        double price = 0;
        const char *item = "";
        switch (choice) {
            case 1: item = "Lamination - SHORT BONDPAPER"; price = 60; break;
            case 2: item = "Lamination - LONG BONDPAPER";  price = 70; break;
            case 3: item = "Lamination - LEGAL BONDPAPER"; price = 70; break;
            case 4: item = "Lamination - 65mm x 95mm";     price = 20; break;
            case 5: item = "Lamination - 80mm x 110mm";    price = 25; break;
            case 6: item = "Lamination - 95mm x 115mm";    price = 30; break;
        }

        int qty = getValidInteger("Quantity: ", 1, 999);
        AddToCart(item, price, qty);
    }
}

void RushID(void) {
    int choice;

    while (1) {
        Header();
        PrintCentered("=================\n");
        PrintCentered(" RUSH ID PACKAGES\n");
        PrintCentered("=================\n\n");

char msg[100];
        snprintf(msg, sizeof(msg), "CART TOTAL: P%.2f (%d items)", cartTotal, cartItemCount);
        PrintCentered(msg);
        printf("\n");
        PrintCentered("[1]  8-1x1                          P70\n");
        PrintCentered("[2]  6-2x2                          P70\n");
        PrintCentered("[3]  6-TESDA PP                     P70\n");
        PrintCentered("[4]  6-PRC PP                       P70\n");
        PrintCentered("[5]  3-2x2 & 3-1x1                  P70\n");
        PrintCentered("[6]  3-PP size & 3-1x1              P70\n");
        PrintCentered("[7]  3-2x2 & 3-PP                   P70\n");
        PrintCentered("[8]  ASA                            P70\n");
        PrintCentered("[9]  CSC                            P100\n");
        PrintCentered("[10] 2-2x2, 2-PP & 2-1x1            P100\n");
        PrintCentered("[11] 2-2x2, 4-PP & 4-1x1            P120\n\n");
        PrintCentered("[0]  BACK TO PHOTO PRINTING\n\n");

        choice = getValidInteger(" Choose: ", 0, 11);

        if (choice == 0) return;

        double price = 0;
        const char *item = "";
        switch (choice) {
            case 1:  item = "Rush ID - 8-1x1";                     price = 70;  break;
            case 2:  item = "Rush ID - 6-2x2";                     price = 70;  break;
            case 3:  item = "Rush ID - 6-TESDA PP";                price = 70;  break;
            case 4:  item = "Rush ID - 6-PRC PP";                  price = 70;  break;
            case 5:  item = "Rush ID - 3-2x2 & 3-1x1";             price = 70;  break;
            case 6:  item = "Rush ID - 3-PP size & 3-1x1";         price = 70;  break;
            case 7:  item = "Rush ID - 3-2x2 & 3-PP";              price = 70;  break;
            case 8:  item = "Rush ID - ASA";                       price = 70;  break;
            case 9:  item = "Rush ID - CSC";                       price = 100; break;
            case 10: item = "Rush ID - 2-2x2, 2-PP & 2-1x1";       price = 100; break;
            case 11: item = "Rush ID - 2-2x2, 4-PP & 4-1x1";       price = 120; break;
        }

        int qty = getValidInteger(" Quantity: ", 1, 999);
        AddToCart(item, price, qty);
    }
}

void Visa(void) {
    int choice;

    while (1) {
        Header();
        PrintCentered("========================");
        PrintCentered("\n");
        PrintCentered(" VISA/PASSPORT PACKAGES");
        PrintCentered("\n");
        PrintCentered("========================");
        PrintCentered("\n\n");

        char msg[100];
        snprintf(msg, sizeof(msg), " CART TOTAL: P%.2f (%d items)\n\n", cartTotal, cartItemCount);
        PrintCentered(msg);

        PrintCentered(" [1]  US VISA             P120\n");
        PrintCentered(" [2]  SCHENGEN VISA       P120\n");
        PrintCentered(" [3]  CANADA VISA         P120\n");
        PrintCentered(" [4]  NEW ZEALAND VISA    P120\n");
        PrintCentered(" [5]  CHINESE VISA        P120\n");
        PrintCentered(" [6]  AUSTRALIAN VISA     P120\n");
        PrintCentered(" [7]  JAPANESE VISA       P120\n");
        PrintCentered(" [8]  UK VISA             P120\n\n");
        PrintCentered(" [0]  BACK TO PHOTO PRINTING\n\n");

        choice = getValidInteger(" Choose: ", 0, 8);

        if (choice == 0) return;

        double price = 120;
        const char *item = "";
        switch (choice) {
            case 1:  item = "Visa - US VISA";       break;
            case 2:  item = "Visa - SCHENGEN VISA"; break;
            case 3:  item = "Visa - CANADA VISA";   break;
            case 4:  item = "Visa - NEW ZEALAND VISA"; break;
            case 5:  item = "Visa - CHINESE VISA";  break;
            case 6:  item = "Visa - AUSTRALIAN VISA"; break;
            case 7:  item = "Visa - JAPANESE VISA"; break;
            case 8:  item = "Visa - UK VISA";       break;
        }

        int qty = getValidInteger(" Quantity: ", 1, 999);
        AddToCart(item, price, qty);
    }
}

void OtherSizes(void) {
    int choice;

    while (1) {
        Header();
        PrintCentered("======================\n");
        PrintCentered(" OTHER SPECIFIC SIZES\n");
        PrintCentered("======================\n\n");

        char msg[100];
        snprintf(msg, sizeof(msg), " CART TOTAL: P%.2f (%d items)\n\n", cartTotal, cartItemCount);
        PrintCentered(msg);

        PrintCentered(" [1]  4n1  (1.75\" x 2.5\")      P20\n");
        PrintCentered(" [2]  8n1  (1.75\" x 2.5\")      P40\n");
        PrintCentered(" [3]  2R   (2.5\" x 3.5\")       P20\n");
        PrintCentered(" [4]  3R   (3.5\" x 5\")         P12\n");
        PrintCentered(" [5]  4R   (4\" x 6\")           P15\n");
        PrintCentered(" [6]  5R   (5\" x 7\")           P25\n");
        PrintCentered(" [7]  6R   (6\" x 8\")           P65\n");
        PrintCentered(" [8]  8R   (8\" x 10\")          P75\n");
        PrintCentered(" [9]  A4   (8.3\" x 11.7\")      P80\n");
        PrintCentered(" [10] S8R  (8\" x 12\")          P95\n");
        PrintCentered(" [11] 10R  (10\" x 12\")        P150\n");
        PrintCentered(" [12] 11R  (11\" x 14\")        P180\n");
        PrintCentered(" [13] 12R  (12\" x 18\")        P200\n");
        PrintCentered(" [14] 20\" x 24\"               P400\n\n");
        PrintCentered(" [0]  BACK TO PHOTO PRINTING\n\n");

        choice = getValidInteger(" Choose: ", 0, 14);

        if (choice == 0) return;

        double price = 0;
        const char *item = "";

        switch (choice) {
            case 1:  item = "Photo - 4n1";   price = 20;  break;
            case 2:  item = "Photo - 8n1";   price = 40;  break;
            case 3:  item = "Photo - 2R";    price = 20;  break;
            case 4:  item = "Photo - 3R";    price = 12;  break;
            case 5:  item = "Photo - 4R";    price = 15;  break;
            case 6:  item = "Photo - 5R";    price = 25;  break;
            case 7:  item = "Photo - 6R";    price = 65;  break;
            case 8:  item = "Photo - 8R";    price = 75;  break;
            case 9:  item = "Photo - A4";    price = 80;  break;
            case 10: item = "Photo - S8R";   price = 95;  break;
            case 11: item = "Photo - 10R";   price = 150; break;
            case 12: item = "Photo - 11R";   price = 180; break;
            case 13: item = "Photo - 12R";   price = 200; break;
            case 14: item = "Photo - 20\" x 24\""; price = 400; break;
        }

        int qty = getValidInteger(" Quantity: ", 1, 999);
        AddToCart(item, price, qty);
    }
}

void TarpulinPrinting(void) {
    int choice;

    while (1) {
        Header();
        PrintCentered("==================");
        PrintCentered("\n");
        PrintCentered(" TARPULIN SIZES");
        PrintCentered("\n");
        PrintCentered("=================");
        PrintCentered("\n\n");

        char msg[100];
        snprintf(msg, sizeof(msg), " CART TOTAL: P%.2f (%d items)\n\n", cartTotal, cartItemCount);
        PrintCentered(msg);

        PrintCentered(" [1]  2' x 3'          P150\n");
        PrintCentered(" [2]  3' x 4'          P200\n");
        PrintCentered(" [3]  4' x 5'          P250\n");
        PrintCentered(" [4]  5' x 6'          P300\n");
        PrintCentered(" [5]  6' x 7'          P350\n");
        PrintCentered(" [6]  6' x 8'          P400\n");
        PrintCentered(" [7]  CUSTOM SIZE\n\n");
        PrintCentered(" [0]  BACK TO SERVICES\n\n");

        choice = getValidInteger(" Choose: ", 0, 7);

        if (choice == 0) return;

        double price = 0;
        char item[128];

        switch (choice) {
            case 1: strcpy(item, "Tarpulin - 2'x3'"); price = 150; break;
            case 2: strcpy(item, "Tarpulin - 3'x4'"); price = 200; break;
            case 3: strcpy(item, "Tarpulin - 4'x5'"); price = 250; break;
            case 4: strcpy(item, "Tarpulin - 5'x6'"); price = 300; break;
            case 5: strcpy(item, "Tarpulin - 6'x7'"); price = 350; break;
            case 6: strcpy(item, "Tarpulin - 6'x8'"); price = 400; break;
            case 7: {
                PrintCentered(" Type 0 to cancel..");
                printf("\n\n");
                double width = getValidInteger(" Enter WIDTH in feet: ", 0, 99);

                double height = getValidInteger("Enter HEIGHT in feet(7 ft MAX): ", 0, 7);

                if (width||height == 0){
                    return TarpulinPrinting();
                }

                double sqft = width * height;
                price = sqft * 18.0; // price per sqft
                snprintf(item, sizeof(item), "Tarpulin - %.1f'x%.1f' (%.2f sqft)", width, height, sqft);
                Header();
                printf("\n Price for custom tarp: P%.2f\n\n", price);
                PrintCentered(" Press any key to continue...");
                getch();
                break;
            }
        }

        int qty = getValidInteger(" Quantity: ", 1, 999);
        AddToCart(item, price, qty);
    }
}

void Scanning(void) {
    int choice;

    while (1) {
        Header();
        PrintCentered("       ==========\n");
        PrintCentered("        SCANNING\n");
        PrintCentered("       ==========\n\n");
        printf("\n");

        char msg[100];
        snprintf(msg, sizeof(msg), " CART TOTAL: P%.2f (%d items)\n\n", cartTotal, cartItemCount);
        PrintCentered(msg);

        PrintCentered(" [1] P20 PER SCAN\n\n");
        printf("\n");
        PrintCentered(" [0] BACK TO SERVICES\n\n");
        printf("\n\n");

        choice = getValidInteger(" Choose: ", 0, 1);

        if (choice == 0) return;

        if (choice == 1) {
            PrintCentered(" Enter 0 if you want to cancel scanning");
            int qty = getValidInteger(" Number of scans: ", 0, 999);
            if(qty == 0) {

            PrintCentered(" EXITING SCANNING...\n");
            PrintCentered(" Press Enter to Continue");
            getch();
            return NewOrder();
            }
            AddToCart("Scanning", 20.0, qty);


        }
    }
}

void PhotoPrinting(void) {
    int choice;
    while (1) {
        Header();
        PrintCentered("====================\n");
        PrintCentered(" PHOTO PRINTING\n");
        PrintCentered("====================\n\n");
        printf("\n\n");

        char msg[100];
        snprintf(msg, sizeof(msg), " CART TOTAL: P%.2f (%d items)\n\n", cartTotal, cartItemCount);
        PrintCentered(msg);

        PrintCentered(" [1] RUSH ID\n");
        printf("\n");
        PrintCentered(" [2] VISA\n");
        printf("\n");
        PrintCentered(" [3] OTHER SIZES\n\n");
        printf("\n\n");
        PrintCentered(" [0] BACK TO SERVICES\n\n");
        printf("\n");

        choice = getValidInteger(" Choose: ", 0, 3);

        if (choice == 0) return;

        switch (choice) {
            case 1: RushID(); break;
            case 2: Visa(); break;
            case 3: OtherSizes(); break;
        }
    }
}

// ==================== SEARCH / NEW ORDER ====================

#include <dirent.h>


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>


// Structure to hold receipt info for sorting
typedef struct {
    char date[16]; // YYYY-MM-DD
    char customer[64];
    char services[128];
    char total[32];
} Receipt;

int compareReceipts(const void *a, const void *b) {
    // Sort by date descending
    return -strcmp(((Receipt *)a)->date, ((Receipt *)b)->date);
}

void SearchTransaction(void) {
    while (1) {
        Header();
        PrintCentered("=====================");
        PrintCentered(" TARPAULIN HISTORY");
        PrintCentered("=====================\n\n");

        PrintCentered("Select an option:\n\n");
        PrintCentered("[1] All Tarpaulin orders\n\n");
        printf("\n");
        PrintCentered("[2] Claimed Tarpaulin orders\n\n");
        printf("\n");
        PrintCentered("[3] Pending Tarpaulin orders\n\n");
        printf("\n\n");
        PrintCentered("[0] Exit\n\n");

        int choice = getValidInteger("Enter choice: ", 0, 3);
        if (choice == 0) return; // Exit option

        FILE *fp = fopen("tarp_orders.txt", "r");
        if (!fp) {
            PrintCentered("No tarpaulin order file found.");
        } else {
            char line[1024];
            int found = 0;

            PrintCentered("\nTARPAULIN ORDERS:\n");
            printf("%-10s %-15s %-12s %-30s %-8s %-8s\n", "CODE", "CUSTOMER", "PHONE", "SERVICES", "AMOUNT", "STATUS");
            printf("--------------------------------------------------------------------------------------\n");

            while (fgets(line, sizeof(line), fp)) {
                char code[128], customer[256], phone[64], amountStr[64], services[512], status[64];
                sscanf(line, "%127[^|]|%255[^|]|%63[^|]|%63[^|]|%511[^|]|%63[^\n]", code, customer, phone, amountStr, services, status);

                if (choice == 1 ||
                   (choice == 2 && strcmp(status, "CLAIMED") == 0) ||
                   (choice == 3 && strcmp(status, "PENDING") == 0)) {
                    printf("%-10s %-15s %-12s %-30s %-8s %-8s\n",
                           code, customer, phone, services, amountStr, status);
                    found = 1;
                }
            }

            if (!found) PrintCentered("No matching tarpaulin orders found.");
            fclose(fp);
        }

        printf("\nPress any key to continue...");
        getch();
        system("cls");
    }
}




void NewOrder(void) {
    int choice;

    while (1) {
        Header();
        PrintCentered("====================");
        PrintCentered("\n");
        PrintCentered(" SERVICE MENU");
        PrintCentered("\n\n");
        PrintCentered("====================");
        PrintCentered("\n\n");

        char msg[100];
        snprintf(msg, sizeof(msg), " CART TOTAL: P%.2f (%d items)\n\n", cartTotal, cartItemCount);
        PrintCentered(msg);



        PrintCentered(" [1] PHOTO PRINTING");
        printf("\n");
        PrintCentered(" [2] TARPULIN PRINTING");
        printf("\n");
        PrintCentered(" [3] LAMINATION");
        printf("\n");
        PrintCentered(" [4] SCANNING");
        printf("\n");
        PrintCentered(" [5] VIEW CART");
        printf("\n");
        PrintCentered(" [6] CHECKOUT");
        printf("\n");
        PrintCentered(" [7] EDIT CART");
        printf("\n\n");
        PrintCentered(" [0] BACK TO MAIN MENU\n\n");

        choice = getValidInteger(" Choose: ", 0, 7);

        switch (choice) {
            case 0: return;
            case 1: PhotoPrinting(); break;
            case 2: TarpulinPrinting(); break;
            case 3: Lamination(); break;
            case 4: Scanning(); break;
            case 5: ViewCart(); break;
            case 6: Checkout(); break;

            case 7:
                EditCart();
                Header();
                printf("\n Press Enter  continue...\n");
                getch();
                break;
        }
    }
}

// ==================== CASHIER MANAGEMENT ====================

void ViewAllCashiers(void) {
    Header();
    PrintCentered("====================");
    PrintCentered("   ALL CASHIERS");
    PrintCentered("====================");
    printf("\n\n");

    FILE *fp = fopen("CashierLoginCredentials.txt", "r");
    if (fp == NULL) {
        PrintCentered("Error: Cannot open Cashier Credentials file.");
        PrintCentered("No cashiers registered yet.");
        printf("\n");
        PrintCentered("Press any key...");
        getch();
        return;
    }

    char line[200];
    int count = 0;

    PrintCentered("NO.  USERNAME");
    PrintCentered("--------------------------------");

    while (fgets(line, sizeof(line), fp)) {
        char username[50], password[50];

        if (sscanf(line, "%[^,],%s", username, password) == 2) {
            count++;
            char display[100];
            snprintf(display, sizeof(display), "%-4d %s", count, username);
            PrintCentered(display);
        }
    }

    fclose(fp);

    PrintCentered("--------------------------------");
    char total[100];
    snprintf(total, sizeof(total), "Total Cashiers: %d", count);
    PrintCentered(total);
    printf("\n");
    PrintCentered("Press any key...");
    getch();
}

void AddNewCashier(void) {
    Header();
    PrintCentered("====================");
    PrintCentered("   ADD NEW CASHIER");
    PrintCentered("====================");
    printf("\n\n");

    char username[50], password[50], confirmPassword[50];

    getValidString("New Cashier Username: ", username, sizeof(username));

    FILE *check = fopen("CashierLoginCredentials.txt", "r");
    if (check) {
        char line[200];
        char existingUser[50];

        while (fgets(line, sizeof(line), check)) {
            if (sscanf(line, "%[^,]", existingUser) == 1) {
                if (strcmp(username, existingUser) == 0) {
                    fclose(check);
                    Header();
                    PrintCentered("ERROR: Username already exists!");
                    PrintCentered("Please choose a different username.");
                    printf("\n");
                    PrintCentered("Press any key...");
                    getch();
                    return;
                }
            }
        }
        fclose(check);
    }


    PrintCenteredPrompt("New Password: ");
    scanf("%s", &password);
    PrintCenteredPrompt("Confirm Password: ");
    scanf("%s", &confirmPassword);

    if (strcmp(password, confirmPassword) != 0) {
        Header();
        PrintCentered("ERROR: Passwords do not match!");
        printf("\n");
        PrintCentered("Press any key...");
        getch();
        return;
    }

    if (strlen(password) < 4) {
        Header();
        PrintCentered("ERROR: Password too short!");
        PrintCentered("Password must be at least 4 characters.");
        printf("\n");
        PrintCentered("Press any key...");
        getch();
        return;
    }

    FILE *fp = fopen("CashierLoginCredentials.txt", "a");
    if (!fp) {
        Header();
        PrintCentered("ERROR: Cannot save cashier credentials.");
        printf("\n");
        PrintCentered("Press any key...");
        getch();
        return;
    }

    fprintf(fp, "\n%s,%s", username, password);
    fclose(fp);

    Header();
    PrintCentered(" SUCCESS!");
    char msg[100];
    snprintf(msg, sizeof(msg), "Cashier '%s' added successfully!", username);
    PrintCentered(msg);
    printf("\n");
    PrintCentered("Press any key...");
    getch();
}

void RemoveCashier(void) {
    Header();
    PrintCentered("====================");
    PrintCentered("   REMOVE CASHIER");
    PrintCentered("====================");
    printf("\n\n");


    FILE *fp = fopen("CashierLoginCredentials.txt", "r");
    if (fp == NULL) {
        PrintCentered("Error: Cannot open Cashier Credentials file.");
        PrintCentered("No cashiers to remove.");
        printf("\n");
        PrintCentered("Press any key...");
        getch();
        return;
    }

    char line[200];
    char usernames[50][50];
    int count = 0;

    PrintCentered("NO.  USERNAME");
    PrintCentered("--------------------------------");

    while (fgets(line, sizeof(line), fp) && count < 50) {
        char username[50], password[50];

        if (sscanf(line, "%[^,],%s", username, password) == 2) {
            strcpy(usernames[count], username);
            char display[100];
            snprintf(display, sizeof(display), "%-4d %s", count + 1, username);
            PrintCentered(display);
            count++;
        }
    }
    fclose(fp);

    PrintCentered("--------------------------------");
    printf("\n");

    if (count == 0) {
        PrintCentered("No cashiers to remove.");
        printf("\n");
        PrintCentered("Press any key...");
        getch();
        return;
    }

    char removeUser[50];
    getValidString("Enter username to remove (or type '0' to cancel): ", removeUser, sizeof(removeUser));

    if (strcmp(removeUser, "0") == 0) {
        return;
    }

    Header();
    char confirmMsg[100];
    snprintf(confirmMsg, sizeof(confirmMsg), " WARNING: Remove cashier '%s'?", removeUser);
    PrintCentered(confirmMsg);
    PrintCentered("This action cannot be undone!");
    printf("\n");

    char confirm[10];
    getValidString("Type 'YES' to confirm: ", confirm, sizeof(confirm));

    if (strcmp(confirm, "YES") != 0) {
        PrintCentered("Removal cancelled.");
        printf("\n");
        PrintCentered("Press any key...");
        getch();
        return;
    }

    FILE *in = fopen("CashierLoginCredentials.txt", "r");
    FILE *out = fopen("CashierLoginCredentials_tmp.txt", "w");

    if (!in || !out) {
        if (in) fclose(in);
        if (out) fclose(out);
        PrintCentered("ERROR: Cannot access credentials file.");
        printf("\n");
        PrintCentered("Press any key...");
        getch();
        return;
    }

    int found = 0;
    while (fgets(line, sizeof(line), in)) {
        char username[50], password[50];

        if (sscanf(line, "%[^,],%s", username, password) == 2) {
            if (strcmp(username, removeUser) == 0) {
                found = 1;
            } else {
                fprintf(out, "%s,%s\n", username, password);
            }
        }
    }

    fclose(in);
    fclose(out);

    remove("CashierLoginCredentials.txt");
    rename("CashierLoginCredentials_tmp.txt", "CashierLoginCredentials.txt");

    Header();
    if (found) {
        PrintCentered(" SUCCESS!");
        snprintf(confirmMsg, sizeof(confirmMsg), "Cashier '%s' removed successfully!", removeUser);
        PrintCentered(confirmMsg);
    } else {
        PrintCentered("ERROR: Cashier not found!");
    }
    printf("\n");
    PrintCentered("Press any key...");
    getch();
}

void ChangeCashierPassword(void) {
    Header();
    PrintCentered("====================");
    PrintCentered(" CHANGE CASHIER PWD");
    PrintCentered("====================");
    printf("\n");

    char username[50];
    getValidString("Enter cashier username: ", username, sizeof(username));

    FILE *fp = fopen("CashierLoginCredentials.txt", "r");
    if (!fp) {
        PrintCentered("ERROR: Cannot open credentials file.");
        printf("\n");
        PrintCentered("Press any key...");
        getch();
        return;
    }

    char line[200];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        char fileUser[50];
        if (sscanf(line, "%[^,]", fileUser) == 1) {
            if (strcmp(username, fileUser) == 0) {
                found = 1;
                break;
            }
        }
    }
    fclose(fp);

    if (!found) {
        Header();
        PrintCentered("ERROR: Cashier not found!");
        printf("\n");
        PrintCentered("Press any key...");
        getch();
        return;
    }

    char newPassword[50], confirmPassword[50];

    PrintCenteredPrompt("New Password: ");
    inputPassword(newPassword, sizeof(newPassword));

    PrintCenteredPrompt("Confirm Password: ");
    inputPassword(confirmPassword, sizeof(confirmPassword));

    if (strcmp(newPassword, confirmPassword) != 0) {
        Header();
        PrintCentered("ERROR: Passwords do not match!");
        printf("\n");
        PrintCentered("Press any key...");
        getch();
        return;
    }

    if (strlen(newPassword) < 4) {
        Header();
        PrintCentered("ERROR: Password too short!");
        PrintCentered("Password must be at least 4 characters.");
        printf("\n");
        PrintCentered("Press any key...");
        getch();
        return;
    }

    // Update password in file
    FILE *in = fopen("CashierLoginCredentials.txt", "r");
    FILE *out = fopen("CashierLoginCredentials_tmp.txt", "w");

    if (!in || !out) {
        if (in) fclose(in);
        if (out) fclose(out);
        PrintCentered("ERROR: Cannot access credentials file.");
        printf("\n");
        PrintCentered("Press any key...");
        getch();
        return;
    }

    while (fgets(line, sizeof(line), in)) {
        char fileUser[50], filePass[50];

        if (sscanf(line, "%[^,],%s", fileUser, filePass) == 2) {
            if (strcmp(fileUser, username) == 0) {
                fprintf(out, "%s,%s\n", fileUser, newPassword);
            } else {
                fprintf(out, "%s,%s\n", fileUser, filePass);
            }
        }
    }

    fclose(in);
    fclose(out);

    remove("CashierLoginCredentials.txt");
    rename("CashierLoginCredentials_tmp.txt", "CashierLoginCredentials.txt");

    Header();
    PrintCentered(" SUCCESS!");
    char msg[100];
    snprintf(msg, sizeof(msg), "Password changed for cashier '%s'!", username);
    PrintCentered(msg);
    printf("\n");
    PrintCentered("Press any key...");
    getch();
}

void ManageCashiers(void) {
    int choice;

    while (1) {
        Header();
        PrintCentered("====================");
        PrintCentered(" CASHIER MANAGEMENT");
        PrintCentered("====================");
        printf("\n\n");

        PrintCentered("[1] VIEW ALL CASHIERS");
        printf("\n");
        PrintCentered("[2] ADD NEW CASHIER");
        printf("\n");
        PrintCentered("[3] REMOVE CASHIER");
        printf("\n");
        PrintCentered("[4] CHANGE CASHIER PASSWORD");
        printf("\n");
        PrintCentered("[0] BACK TO ADMIN MENU");
        printf("\n");

        choice = getValidInteger("Choose: ", 0, 4);

        switch (choice) {
            case 0: return;
            case 1: ViewAllCashiers(); break;
            case 2: AddNewCashier(); break;
            case 3: RemoveCashier(); break;
            case 4: ChangeCashierPassword(); break;
        }
    }
}

// ==================== LOGIN & MENU FUNCTIONS ====================

void AdminMenu(void) {
    int choice;
    system("cls");

    while (1) {
        Header();
        PrintCentered("====================\n");
        PrintCentered(" ADMIN MENU\n");
        PrintCentered("====================");
        printf("\n\n");

        PrintCentered(" [1] NEW ORDER");
        printf("\n");
        PrintCentered(" [2] PREVIOUS TRANSACTIONS");
        printf("\n");
        PrintCentered(" [3] CLAIM TARP ORDER");
        printf("\n");
        PrintCentered(" [4] CASHIER MANAGEMENT");
        printf("\n");
        PrintCentered(" [0] LOGOUT\n\n");

        choice = getValidInteger(" Choose: ", 0, 4);

        switch (choice) {
            case 0: return;
            case 1: NewOrder(); break;
            case 2: SearchTransaction(); break;
            case 3: ClaimTarpOrder(); break;
            case 4: ManageCashiers(); break;
        }
    }
}

void CashierMenu(void) {
    int choice;

    while (1) {
        Header();
        PrintCentered("====================\n");
        PrintCentered(" CASHIER MENU\n");
        PrintCentered("====================\n\n");

        PrintCentered(" [1] NEW ORDER\n");
        printf("\n");
        PrintCentered(" [2] PREVIOUS TRANSACTIONS\n");
        printf("\n");
        PrintCentered(" [3] CLAIM TARP ORDER\n");
        printf("\n");
        PrintCentered(" [0] LOGOUT\n\n");

        choice = getValidInteger(" Choose: ", 0, 3);

        switch (choice) {
            case 0: return;
            case 1: NewOrder(); break;
            case 2: SearchTransaction(); break;
            case 3: ClaimTarpOrder(); break;
        }
    }
}

void AdminLogin(void) {
    int LoginAttempts = 0;
    char username[50], password[50];
    char fileuser[50], filepass[50];
    char line[200];
    int match;

    while (LoginAttempts < 4) {

        system("cls");
        Header();
        PrintCentered("=====================\n");
        PrintCentered(" ADMINISTRATOR LOGIN\n");
        PrintCentered("=====================\n\n");
        printf("\n");
        PrintCentered(" Type '0' to exit...\n\n");

        fflush(stdin);

        printf("\n\n\n                                 Username: ");
        if (fgets(username, sizeof(username), stdin) == NULL) {
            printf("\n Error reading input.\n");
            getch();
            return;
        }
        username[strcspn(username, "\n")] = 0;

        if (strcmp(username, "0") == 0) {
            PrintCentered("\n Exiting Administrator Login...\n");
            system("pause");
            Login();
            return;
        }

        printf("\n                                 Password: ");
        inputPassword(password, sizeof(password));
        password[strcspn(password, "\n")] = 0;

        FILE *fp = fopen("AdminLoginCredentials.txt", "r");
if (fp == NULL) {
    Header();
    PrintCentered("========================================");
    PrintCentered(" NO CASHIERS REGISTERED");
    PrintCentered("========================================");
    printf("\n");
    PrintCentered("Please contact the administrator");
    PrintCentered("to create a cashier account.");
    printf("\n");
    PrintCentered("Press any key to return...");
    getch();
    Login();
    return;
}

        int match = 0;

        while (fgets(line, sizeof(line), fp)) {

            if (sscanf(line, "%[^,],%s", fileuser, filepass) == 2) {

                if (strcmp(username, fileuser) == 0 &&
                    strcmp(password, filepass) == 0){
                    match = 1;
                    break;
                    }
                }
            }

        fclose(fp);

        if (match) {
            strcpy(loggedInUser, username);

            system("cls");
            Header();
            PrintCentered("Logged in successfully!\n");
            PrintCentered("Welcome Admin!!\n");
            system("pause");

            AdminMenu();
            return;
        }

        LoginAttempts++;
        Header();
        char msg[100];
        snprintf(msg, sizeof(msg), "\n  Invalid credentials! Attempts Left: %d\n", 4 - LoginAttempts);
        PrintCentered(msg);
        system("pause");
    }

    if (LoginAttempts == 4) {
        PrintCentered(" Maximum Attempts Reached... Exiting Admin Login\n");
        Login();
        return;
    }
}


void CashierLogin(void) {
    int LoginAttempts = 0;
    char username[50], password[50];
    char fileuser[50], filepass[50];
    char line[200];
    int match;

    while (LoginAttempts < 4) {

        system("cls");
        Header();
        PrintCentered("====================\n");
        PrintCentered(" CASHIER LOGIN\n");
        PrintCentered("====================\n");
        PrintCentered("Type '0' to exit\n\n");

        fflush(stdin);

        printf("\n\n\n                                 Username: ");
        if (fgets(username, sizeof(username), stdin) == NULL) {
            printf("\n Error reading input.\n");
            getch();
            return;
        }
        username[strcspn(username, "\n")] = 0;

        if (strcmp(username, "0") == 0) {
            printf("\n Exiting Cashier Login...\n");
            system("pause");
            Login();
            return;
        }

        printf("\n                                 Password: ");
        inputPassword(password, sizeof(password));
        password[strcspn(password, "\n")] = 0;

            FILE *fp = fopen("CashierLoginCredentials.txt", "r");
            if (fp == NULL) {
                PrintCentered("Error: cannot open CashierCredentials file.\n");
                system("pause");
                Login();
                return;
        }

        int match = 0;

        while (fgets(line, sizeof(line), fp)) {


            if (sscanf(line, "%[^,],%s", fileuser, filepass) == 2) {

                if (strcmp(username, fileuser) == 0 &&
                    strcmp(password, filepass) == 0){
                    match = 1;
                    break;
                    }
                }
            }

        fclose(fp);

        if (match) {
            strcpy(loggedInUser, username);

            system("cls");
            PrintCentered("Logged in successfully!\n");
            PrintCentered("Welcome Cashier!!\n");
            system("pause");

            CashierMenu();
            return;
        }

        LoginAttempts++;
        Header();
        char msg[100];
        snprintf(msg, sizeof(msg), " Invalid credentials! Attempts Left: %d\n", 4 - LoginAttempts);
        PrintCentered(msg);
        system("pause");
    }

    if (LoginAttempts == 4) {
        PrintCentered(" Maximum Attempts Reached... Exiting Cashier Login\n");
        Login();
        return;
    }
}

void Login(void) {
    int choice;

    while (1) {
        Header();
        PrintCentered("====================");
        PrintCentered("\n");
        PrintCentered(" LOGIN");
        PrintCentered("\n");
        PrintCentered("====================");
        printf("\n\n");

        PrintCentered("[1] ADMIN\n\n");
        printf("\n");
        PrintCentered("[2] CASHIER\n\n");
        printf("\n");
        PrintCentered("[0] EXIT PROGRAM\n\n");

        choice = getValidInteger("Choose: ", 0, 2);

        switch (choice) {
            case 0:
                Header();
                PrintCentered("\n Thank you for using our system!");
                printf("\n");
                PrintCentered(" Press any key to exit...");
                getch();
                exit(0);
                exit(0);
            case 1: AdminLogin(); break;
            case 2: CashierLogin(); break;
        }
    }
}

// ==================== MAIN ====================

int main(void) {
    srand((unsigned int)time(NULL));
    setGreenText();
    Header();
    PrintCentered("   +====   ||     ||    ||    ||   ========   ========   +======  =====");
    PrintCentered("\n");
    PrintCentered("    ||       ||     ||    ||    ||      ||         ||      ||       ||   \\\ ");
    PrintCentered("\n");
    PrintCentered("    ||       ||     ||    ||    ||      ||         ||      ||       ||   // ");
    PrintCentered("\n");
    PrintCentered("   +===+    =========    ||    ||      ||         ||      ======   ||  // ");
    PrintCentered("\n");
    PrintCentered("      ||   ||     ||    ||    ||      ||         ||      ||       || \\\ ");
    PrintCentered("\n");
    PrintCentered("       ||   ||     ||    ||    ||      ||         ||      ||       ||  \\\ ");
    PrintCentered("\n");
    PrintCentered("   ====+    ||     ||    +======+      ||         ||      +======  ||   \\\ ");
    PrintCentered("\n");
    PrintCentered("   +====      ======      +======    ======    =======      ===     ||");
    PrintCentered("\n");
    PrintCentered("   ||   \\       ||       ||            ||        ||        // \\     ||");
    PrintCentered("\n");
    PrintCentered("   ||    \\      ||       ||            ||        ||       //   \\    ||");
    PrintCentered("\n");
    PrintCentered("   ||    ||     ||       ||            ||        ||       ||   ||   ||");
    PrintCentered("\n");
    PrintCentered("   ||    ||     ||       ||    ==+     ||        ||       ||+++||   ||");
    PrintCentered("\n");
    PrintCentered("   ||    //     ||       ||     ||     ||        ||       ||   ||   ||");
    PrintCentered("\n");
    PrintCentered("   ||   //      ||       ||     ||     ||        ||       ||   ||   ||");
    PrintCentered("\n");
    PrintCentered("        +====+     ======     +=======   ======       ||       ||   ||   +=====");
    PrintCentered("\n\n");


    PrintCentered("Welcome to Shutter Digital Printing Services!");
    PrintCentered("\n");
    PrintCentered("Press any key to start...");
    getch();

    Login();

    return 0;
}
