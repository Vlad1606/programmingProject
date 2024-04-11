
#include "shop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void login();

int main(int argc, char* argv[]) {
    printf("Welcome to ShopApp!\n");
    login();

    int choice;
    do {
        printf("1. Place Order\n2. Sell Product\n3. View Orders\n4. Search Products\n0. Exit\nEnter choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                placeOrder();
                break;
            case 2:
                sellProduct();
                break;
            case 3:
                viewPlacedOrders(current_user->username);
                break;
            case 4:
            {
                searchProducts();
                break;
            }
                break;
            default:
                break;
        }
    } while (choice != 0);

    return 0;
}
