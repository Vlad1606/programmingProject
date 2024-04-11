#include "shop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>   

Person* current_user = NULL;
Product* products_db = NULL;
int num_products = 0;

void signup() {
    char username[100], password[100], line[256];
    int exists = 0;
    printf("Signup - Enter username: ");
    scanf("%99s", username);
    printf("Choose a password: ");
    scanf("%99s", password);
    FILE *file = fopen("accounts.csv", "r+");
    while (fgets(line, sizeof(line), file)) {
        char storedUser[100];
        if (sscanf(line, "%99[^,]", storedUser) == 1) {
            if (strcmp(username, storedUser) == 0) {
                printf("Username already exists. Try again.\n");
                exists = 1;
                break;
            }
        }
    }
    if (!exists) {
        fseek(file, 0, SEEK_END);
        fprintf(file, "%s,%s\n", username, password);
        printf("Account created successfully! Please log in.\n");
        fclose(file);
        login();
    } else {
        fclose(file);
        signup();
    }
}

void login() {
    char username[100], password[100], line[256];
    int loginSuccess = 0;
    
    printf("Enter username: ");
    scanf("%99s", username);
    printf("Enter password: ");
    scanf("%99s", password);

    FILE *file = fopen("accounts.csv", "r");

    while (fgets(line, sizeof(line), file) && !loginSuccess) {
        char storedUser[100], storedPwd[100];
        if (sscanf(line, "%99[^,],%99[^\n]", storedUser, storedPwd) == 2) {
            if (strcmp(username, storedUser) == 0) {
                if (strcmp(password, storedPwd) == 0) {
                    loginSuccess = 1;
                    current_user = malloc(sizeof(Person));
                    current_user->username = strdup(username);
                    printf("Login successful. Welcome, %s!\n", username);
                } else {
                    printf("Incorrect password, try again.\n");
                }
            }
        }
    }

    fclose(file);

    if (!loginSuccess) {
        printf("Username not found, redirecting to signup...\n");
        signup();
    }
}

void updateProductsCSV(Product *products, int numProducts) {
    FILE *file = fopen("products.csv", "w");
    for (int i = 0; i < numProducts; i++) {
        fprintf(file, "%s,%d,%.2f\n", products[i].name, products[i].quantity, products[i].price);
    }
    fclose(file);
}

void updateUsersFile(const char *username, int orderNumber) {
    FILE *file = fopen("users.csv", "r+");

    int found = 0;
    char line[1024];
    FILE *tempFile = fopen("temp_users.csv", "w");
    if (!tempFile) {
        perror("Failed to open temporary file for writing");
        fclose(file);
        return;
    }

    while (fgets(line, sizeof(line), file)) {
        char currentUsername[100];
        if (sscanf(line, "%99[^,]", currentUsername) == 1) {
            if (strcmp(currentUsername, username) == 0) {
                fprintf(tempFile, "%s,%d\n", line, orderNumber);
                found = 1;
            } else {
                fprintf(tempFile, "%s", line);
            }
        }
    }

    if (!found) {
        fprintf(tempFile, "%s,%d\n", username, orderNumber);
    }
    fclose(file);
    fclose(tempFile);
    remove("users.csv");
    rename("temp_users.csv", "users.csv");
}

void trimWhitespace(char *str) {
    char *ptr = str;
    int len = strlen(ptr);
    while (isspace(ptr[len - 1])) ptr[--len] = 0;
    while (*ptr && isspace(*ptr)) ++ptr, --len;
    memmove(str, ptr, len + 1);
}

void placeOrder() {
    if (!current_user) {
        printf("No user logged in.\n");
        return;
    }

    Checkout cart;
    cart.numItems = 0;
    char inputName[100];
    int inputQuantity;
    int continueShopping = 1;
    float totalCartPrice = 0.0;

    FILE *file = fopen("products.csv", "r");

    Product products[100];
    int numProducts = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%99[^,],%d,%f", products[numProducts].name, &products[numProducts].quantity, &products[numProducts].price);
        numProducts++;
    }
    fclose(file);

    while (continueShopping) {
        printf("Enter product name: ");
        scanf(" %99[^\n]", inputName);

        int found = 0;
        Product *prod = NULL;

        for (int i = 0; i < numProducts; i++) {
            if (strcmp(products[i].name, inputName) == 0) {
                prod = &products[i];
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("No exact match found. Searching similar products...\n");
            searchProducts();
            continue;
        }

        do {
            printf("Enter quantity: ");
            scanf("%d", &inputQuantity);
            if (inputQuantity > prod->quantity) {
                printf("Not enough in stock. (%d quantity in stock)\n", prod->quantity);
            } else {
                prod->quantity -= inputQuantity; 
                break;
            }
        } while (1);

        updateProductsCSV(products, numProducts);

        strcpy(cart.items[cart.numItems].product_name, prod->name);
        cart.items[cart.numItems].quantity_ordered = inputQuantity;
        cart.numItems++;
        totalCartPrice += prod->price * inputQuantity;

        printf("Send order or continue shopping? (0 = Send, 1 = Continue): ");
        scanf("%d", &continueShopping);
    }

    srand(time(NULL));
    int orderNumber = rand();

    file = fopen("orders.txt", "a");
    fprintf(file, "%s,Order Number: %d\n", current_user->username, orderNumber);
    for (int i = 0; i < cart.numItems; i++) {
        fprintf(file, "%s, %d\n", cart.items[i].product_name, cart.items[i].quantity_ordered);
    }
    fprintf(file, "Total Price: %.2f\n\n", totalCartPrice);
    fclose(file);

    printf("Order placed successfully with order number %d and total price %.2f\n", orderNumber, totalCartPrice);

    updateUsersFile(current_user->username, orderNumber);
}


void sellProduct() {
    char inputName[100];
    int inputQuantity;
    printf("Enter product name: ");
    scanf(" %99[^\n]", inputName); 
    printf("Enter quantity to add: ");
    scanf("%d", &inputQuantity);
    
    FILE *file = fopen("products.csv", "r");

    Product products[100];
    int numProducts = 0;
    int found = 0;
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%99[^,],%d,%f", products[numProducts].name, &products[numProducts].quantity, &products[numProducts].price);
        if (strcmp(products[numProducts].name, inputName) == 0) {
            products[numProducts].quantity += inputQuantity;
            found = 1;
        }
        numProducts++;
    }
    fclose(file);
    if (!found) {
        strcpy(products[numProducts].name, inputName);
        printf("Enter price for new product: ");
        scanf("%f", &products[numProducts].price);
        products[numProducts].quantity = inputQuantity;
        numProducts++;
    }
    file = fopen("products.csv", "w");
    for (int i = 0; i < numProducts; i++) {
        fprintf(file, "%s,%d,%.2f\n", products[i].name, products[i].quantity, products[i].price);
    }
    fclose(file);

    printf("Product updated successfully.\n");
}

void viewPlacedOrders(const char *username) {
    FILE *file = fopen("orders.txt", "r");
    char line[1024];
    int found = 0;
    printf("Orders for %s:\n", username);

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, username)) {
            printf("%s", line);
            while (fgets(line, sizeof(line), file) && strcmp(line, "\n") != 0) {
                printf("%s", line);
            }
            printf("\n");
            found = 1;
        }
    }

    if (!found) {
        printf("No orders found for %s.\n", username);
    }

    fclose(file);
}

void searchProducts() {
    int searchOption;
    char searchName[100];
    float minPrice, maxPrice;
    
    printf("Select search type: 1-Name, 2-Price range, 3-Both: ");
    scanf("%d", &searchOption);

    if (searchOption == 1 || searchOption == 3) {
        printf("Enter product name (or part of it): ");
        scanf("%*c%99[^\n]", searchName);
        for (int i = 0; searchName[i]; i++) {
            searchName[i] = tolower(searchName[i]);
        }
    }
    if (searchOption == 2 || searchOption == 3) {
        printf("Enter minimum price: ");
        scanf("%f", &minPrice);
        printf("Enter maximum price: ");
        scanf("%f", &maxPrice);
    }

    FILE* file = fopen("products.csv", "r");

    char line[256];
    int found = 0;
    printf("Search results:\n");
    while (fgets(line, sizeof(line), file)) {
        char name[100], nameLower[100];
        int quantity;
        float price;     
        sscanf(line, "%99[^,],%d,%f", name, &quantity, &price);
        strcpy(nameLower, name);
        for (int i = 0; nameLower[i]; i++) {
            nameLower[i] = tolower(nameLower[i]);
        }

        int nameMatch = 0, priceMatch = 0;

        if (searchOption == 1 || searchOption == 3) {
            if (strstr(nameLower, searchName) != NULL) {
                nameMatch = 1;
            }
        }

        if (searchOption == 2 || searchOption == 3) {
            if (price >= minPrice && price <= maxPrice) {
                priceMatch = 1;
            }
        }

        if ((searchOption == 1 && nameMatch) ||
            (searchOption == 2 && priceMatch) ||
            (searchOption == 3 && nameMatch && priceMatch)) {
            printf("Name: %s, Quantity: %d, Price: %.2f\n", name, quantity, price); 
            found = 1;
        }
    }

    if (!found) {
        printf("No products found.\n");
    }

    fclose(file);
}