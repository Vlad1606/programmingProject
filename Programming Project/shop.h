
#ifndef SHOP_H
#define SHOP_H

typedef struct {
    char name[100];
    int quantity;
    float price;
} Product;


typedef struct {
    char product_name[100];
    int quantity_ordered;
    float total_price;
} OrderItem;

typedef struct {
    int code;
    char** productNames;
    int* productQuantities;
    int numProducts;
    float paidAmount;
} Order;

typedef struct {
    char* username;
} Person;

typedef struct {
    OrderItem items[100];
    int numItems;
} Checkout;

extern Person* current_user;

void login(); 
void placeOrder();
void sellProduct();
void viewPlacedOrders(const char* username);
void searchProducts();


#endif
