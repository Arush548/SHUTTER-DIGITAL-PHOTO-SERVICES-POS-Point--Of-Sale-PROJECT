#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void PrintReceipt(const char *name, const char *phone, const char *service, double total);
void setGreenText();
void Header();
void Payment(double total);

// ===================== SAFE INPUT =====================
int inputInt() {
    char buf[50];
    int value;
    while (1) {
        if (!fgets(buf, sizeof(buf), stdin)) continue;
        if (sscanf(buf, "%d", &value) == 1) return value;
        printf(" Invalid input. Enter again: ");
    }
}

double inputDouble() {
    char buf[50];
    double value;
    while (1) {
        if (!fgets(buf, sizeof(buf), stdin)) continue;
        if (sscanf(buf, "%lf", &value) == 1) return value;
        printf(" Invalid input. Enter again: ");
    }
}

void inputString(char *str, int size) {
    if (fgets(str, size, stdin)) {
        size_t len = strlen(str);
        if(len > 0 && str[len-1] == '\n') str[len-1] = '\0';
    }
}

// =======================================================

void setGreenText() {
    system("color 0A");
}

void Header() {
    system("cls");
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    printf("+============================================+\n");
    printf("     SHUTTER DIGITAL PRINTING SERVICES   \n");
    printf("     Date: %02d-%02d-%04d   Time: %02d:%02d:%02d\n",
                 t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
                 t->tm_hour, t->tm_min, t->tm_sec);
    printf("+============================================+\n\n");
}

void SaveReceiptToFile(const char *service, double total) {
    system("mkdir C:\\Receipts >nul 2>&1");

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char filename[200];
    sprintf(filename,
        "C:\\Receipts\\receipt_%04d-%02d-%02d_%02d%02d%02d.txt",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec
    );

    FILE *fp = fopen(filename, "w");
    if (!fp) {
        printf("\n ERROR: Cannot save receipt. Check folder permissions.\n");
        getch();
        return;
    }

    fprintf(fp, "==============================================\n");
    fprintf(fp, "      SHUTTER DIGITAL PRINTING SERVICES   \n\n");
    fprintf(fp, "     Ground Flr., Tubigon Comm'l Complex,\n");
    fprintf(fp,     " Centro, Tubigon, Bohol\n");
    fprintf(fp, "     Date: %02d-%02d-%04d   Time: %02d:%02d:%02d\n",
           t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
           t->tm_hour, t->tm_min, t->tm_sec);
    fprintf(fp, "     Contact: 09398567968/09494145607\n");
    fprintf(fp, "---------------------------------------------\n\n");
    fprintf(fp, "              OFFICIAL RECEIPT \n\n\n\n\n\n\n");
    fprintf(fp, "Receipt No: ");
    fprintf(fp, "Service: %s\n", service);
    fprintf(fp, "Total Amount: P%.2f\n\n", total);
    fprintf(fp, "Thank you for your purchase!\n");
    fprintf(fp, "=========================================\n");

    fclose(fp);

    char command[300];
    sprintf(command, "start notepad.exe /p \"%s\"", filename);
    system(command);
}

// ====================== PAYMENT & RECEIPT ======================
void Payment(double total) {
    double cash;
    Header();
    printf("           TOTAL: P%.2f\n\n", total);
    printf(" Enter cash tendered: P");
    cash = inputDouble();

    if(cash >= total) {
        printf("\n           CHANGE: P%.2f\n", cash - total);
        printf("\n       THANK YOU! PAYMENT SUCCESSFUL!\n\n");
        SaveReceiptToFile("Transaction", total);
    } else { 
        printf("\n       INSUFFICIENT CASH!\n\n");
        getch();
        Payment(total);
        return;
    }
    getch();
}

// ====================== LAMINATIONS ============================
void Lamination() {
    double total = 0.0;
    int choice;

    while(1) {
        Header();
        printf("      =================\n");
        printf("       LAMINATION MENU \n");
        printf("      =================\n\n");
        printf(" [1] SHORT BONDPAPER  P60\n");
        printf(" [2] LONG BONDPAPER   P70\n");
        printf(" [3] LEGAL BONDPAPER  P70\n");
        printf(" [4] 65mm x 95mmm     P20\n");
        printf(" [5] 80mm x 110mm     P25\n");
        printf(" [6] 95mm x 115mm     P30\n\n");
        printf(" [0]  DONE -> PAYMENT\n");
        printf(" [99] BACK TO MENU\n\n");
        printf(" Choose: ");
        choice = inputInt();

        if(choice == 99) return;
        if(choice == 0) {
            if(total == 0) {
                printf("\n No items added!\n");
                getch();
                continue;
            }
            Payment(total);
            PrintReceipt("Walk-in", "", "Lamination", total);
            return;
        }

        double price = 0;
        const char *item = "";
        switch(choice) {
            case 1:  item = "SHORT BONDPAPER";            price = 60;  break;
            case 2:  item = "LONG BONDPAPER";             price = 70;  break;
            case 3:  item = "LEGAL BONDPAPER";            price = 70;  break;
            case 4:  item = "65mm x 95mmm";               price = 20;  break;
            case 5:  item = "80mm x 110mm";               price = 25;  break;
            case 6:  item = "95mm x 115mm";               price = 30;  break;
            default:
                printf("\n Invalid choice!\n");
                getch();
                continue;
        }

        int qty;
        printf(" Quantity: ");
        qty = inputInt();
        if(qty < 1) qty = 1;

        total += price * qty;
        Header();
        printf(" Added: %s x %d = P%.2f\n", item, qty, price*qty);
        printf(" New Total: P%.2f\n\n", total);
        printf(" Press any key...");
        getch();
    }
}

// ====================== PHOTO PRINTING ============================
void RushID() {
    double total = 0.0;
    int choice;
    while(1) {
        Header();
        printf("      =================\n");
        printf("       RUSH ID PACKAGES\n");
        printf("      =================\n\n");
        printf(" CURRENT TOTAL: P%.2f\n\n", total);

        printf(" [1]  8-1x1                          P70\n");
        printf(" [2]  6-2x2                          P70\n");
        printf(" [3]  6-TESDA PP                     P70\n");
        printf(" [4]  6-PRC PP                       P70\n");
        printf(" [5]  3-2x2 & 3-1x1                  P70\n");
        printf(" [6]  3-PP size & 3-1x1              P70\n");
        printf(" [7]  3-2x2 & 3-PP                   P70\n");
        printf(" [8]  ASA                            P70\n");
        printf(" [9]  CSC                            P100\n");
        printf(" [10] 2-2x2, 2-PP & 2-1x1            P100\n");
        printf(" [11] 2-2x2, 4-PP & 4-1x1            P120\n\n");
        printf(" [0]  DONE -> PAYMENT\n");
        printf(" [99] BACK TO MENU\n\n");
        printf(" Choose: ");
        choice = inputInt();

        if(choice == 99) return;
        if(choice == 0) {
            if(total == 0) {
                printf("\n No items added!\n");
                getch();
                continue;
            }
            Payment(total);
            PrintReceipt("Walk-in", "", "Rush ID Package(s)", total);
            return;
        }

        double price = 0;
        const char *item = "";
        switch(choice) {
            case 1:  item = "8-1x1";                     price = 70;  break;
            case 2:  item = "6-2x2";                     price = 70;  break;
            case 3:  item = "6-TESDA PP";                price = 70;  break;
            case 4:  item = "6-PRC PP";                  price = 70;  break;
            case 5:  item = "3-2x2 & 3-1x1";             price = 70;  break;
            case 6:  item = "3-PP size & 3-1x1";         price = 70;  break;
            case 7:  item = "3-2x2 & 3-PP";              price = 70;  break;
            case 8:  item = "ASA";                       price = 70;  break;
            case 9:  item = "CSC";                       price = 100; break;
            case 10: item = "2-2x2, 2-PP & 2-1x1";       price = 100; break;
            case 11: item = "2-2x2, 4-PP & 4-1x1";       price = 120; break;
            default:
                printf("\n Invalid choice!\n");
                getch();
                continue;
        }

        int qty;
        printf(" Quantity: ");
        qty = inputInt();
        if(qty < 1) qty = 1;

        total += price * qty;
        Header();
        printf(" Added: %s x %d = P%.2f\n", item, qty, price*qty);
        printf(" New Total: P%.2f\n\n", total);
        printf(" Press any key...");
        getch();
    }
}

// ====================== VISA ============================
void Visa() {
    double total = 0.0;
    int choice;
    while(1) {
        Header();
        printf("      ========================\n");
        printf("       VISA/PASSPORT PACKAGES\n");
        printf("      ========================\n\n");
        printf(" CURRENT TOTAL: P%.2f\n\n", total);

        printf(" [1]  US VISA\n");
        printf(" [2]  SCHENGEN VISA\n");
        printf(" [3]  CANADA VISA\n");
        printf(" [4]  NEW ZEALAND VISA\n");
        printf(" [5]  CHINESE VISA\n");
        printf(" [6]  AUSTRALIAN VISA\n");
        printf(" [7]  JAPANESE VISA\n");
        printf(" [8]  UK VISA\n");
        printf(" [0]  DONE -> PAYMENT\n");
        printf(" [99] BACK TO MENU\n\n");
        printf(" Choose: ");
        choice = inputInt();

        if(choice == 99) return;
        if(choice == 0) {
            if(total == 0) {
                printf("\n No items added!\n");
                getch();
                continue;
            }
            Payment(total);
            PrintReceipt("Walk-in", "", "Visa/Passport Package(s)", total);
            return;
        }

        double price = 0;
        const char *item = "";
        switch(choice) {
            case 1:  item = "US VISA";           price = 120; break;
            case 2:  item = "SCHENGEN VISA";     price = 120; break;
            case 3:  item = "CANADA VISA";       price = 120; break;
            case 4:  item = "NEW ZEALAND VISA";  price = 120; break;
            case 5:  item = "CHINESE VISA";      price = 120; break;
            case 6:  item = "AUSTRALIAN VISA";   price = 120; break;
            case 7:  item = "JAPANESE VISA";     price = 120; break;
            case 8:  item = "UK VISA";           price = 120; break;
            default:
                printf("\n Invalid choice!\n");
                getch();
                continue;
        }

        int qty;
        printf(" Quantity: ");
        qty = inputInt();
        if(qty < 1) qty = 1;

        total += price * qty;
        Header();
        printf(" Added: %s x %d = P%.2f\n", item, qty, price*qty);
        printf(" New Total: P%.2f\n\n", total);
        printf(" Press any key...");
        getch();
    }
}

// ====================== OTHER SIZES ============================
void OtherSizes() {
    double total = 0.0;
    int choice;
    while(1) {
        Header();
        printf("       ======================\n");
        printf("        OTHER SPECIFIC SIZES\n");
        printf("       ======================\n\n");
        printf(" CURRENT TOTAL: P%.2f\n\n", total);

        printf(" [1]  4n1  (1.75'' x 2.5'')      P20\n");
        printf(" [2]  8n1  (1.75'' x 2.5'')      P40\n");
        printf(" [3]  2R   (2.5'' x 3.5'')       P20\n");
        printf(" [4]  3R   (3.5'' x 5'')         P12\n");
        printf(" [5]  4R   (4'' x 6'')           P15\n");
        printf(" [6]  5R   (5'' x 7'')           P25\n");
        printf(" [7]  6R   (6'' x 8'')           P65\n");
        printf(" [8]  8R   (8'' x 10'')          P75\n");
        printf(" [9]  A4   (8.3'' x 11.7'')      P80\n");
        printf(" [10] S8R  (8'' x 12'')          P95\n");
        printf(" [11] 10R  (10'' x 12'')         P150\n");
        printf(" [12] 11R  (11'' x 14'')         P180\n");
        printf(" [13] 12R  (12'' x 18'')         P200\n");
        printf(" [14] 20'' x 24''               P400\n");

        printf(" [0]  DONE -> PAYMENT\n");
        printf(" [99] BACK TO MENU\n\n");

        printf(" Choose: ");
        choice = inputInt();

        if(choice == 99) return;
        if(choice == 0) {
            if(total == 0) {
                printf("\n No items added!\n");
                getch();
                continue;
            }
            Payment(total);
            PrintReceipt("Walk-in", "", "Photo Printing (Other Sizes)", total);
            return;
        }

        double price = 0;
        const char *item = "";

        switch(choice) {
            case 1:  item = "4n1";   price = 20;  break;
            case 2:  item = "8n1";   price = 40;  break;
            case 3:  item = "2R";    price = 20;  break;
            case 4:  item = "3R";    price = 12;  break;
            case 5:  item = "4R";    price = 15;  break;
            case 6:  item = "5R";    price = 25;  break;
            case 7:  item = "6R";    price = 65;  break;
            case 8:  item = "8R";    price = 75;  break;
            case 9:  item = "A4";    price = 80;  break;
            case 10: item = "S8R";   price = 95;  break;
            case 11: item = "10R";   price = 150; break;
            case 12: item = "11R";   price = 180; break;
            case 13: item = "12R";   price = 200; break;
            case 14: item = "20\" x 24\""; price = 400; break;

            default:
                printf("\n Invalid choice!\n");
                getch();
                continue;
        }

        int qty;
        printf(" Quantity: ");
        qty = inputInt();
        if(qty < 1) qty = 1;

        total += price * qty;

        Header();
        printf(" Added: %s x %d = P%.2f\n", item, qty, price * qty);
        printf(" New Total: P%.2f\n\n", total);
        printf(" Press any key...");
        getch();
    }
}

// ====================== PHOTO PRINTING MENU ======================
void PhotoPrinting() {
    int choice;
    Header();
    printf(" [1] RUSH ID\n");
    printf(" [2] VISA\n");
    printf(" [3] OTHER SIZES\n");
    printf(" [0] BACK\n\n");
    printf(" Choose: ");
    choice = inputInt();
    if(choice == 1) RushID();
    if(choice == 2) Visa();
    if(choice == 3) OtherSizes();
}

// ====================== Continue replacing scanf in other modules similarly ======================

// ====================== PRINT RECEIPT ======================
void PrintReceipt(const char *name, const char *phone, const char *service, double total) {
    SaveReceiptToFile(service, total);
}

// ====================== MAIN MENU ======================
void MainMenu() {
    int choice;
    while(1) {
        Header();
        printf(" MAIN MENU\n");
        printf(" [1] Lamination\n");
        printf(" [2] Photo Printing\n");
        printf(" [0] EXIT\n\n");
        printf(" Choose: ");
        choice = inputInt();

        switch(choice) {
            case 1: Lamination(); break;
            case 2: PhotoPrinting(); break;
            case 0: exit(0); break;
            default:
                printf("\n Invalid choice!\n");
                getch();
                break;
        }
    }
}

// ====================== MAIN ======================
int main() {
    setGreenText();
    MainMenu();
    return 0;
}
