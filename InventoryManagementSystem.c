#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_PRODUCTS 0
#define MAX_PRODUCTS 100
#define MAX_PROD_NAME 51
#define MIN_PROD_PRICE 0
#define MAX_PROD_PRICE 100000
#define MIN_PROD_QUANTITY 0
#define MAX_PROD_QUANTITY 1000000
#define MIN_PROD_ID 1
#define MAX_PROD_ID 10000

struct Product {
    int prodId;
    char prodName[MAX_PROD_NAME];
    float prodPrice;
    int prodQuantity;
};

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void printProduct(struct Product p) {
    printf("ID: %d | Name: %s | Price: %.2f | Quantity: %d\n",
           p.prodId, p.prodName, p.prodPrice, p.prodQuantity);
}

void viewAllProducts(struct Product *products, int totalProducts) {
    if (totalProducts == 0) {
        printf("No products available.\n");
        return;
    }
    printf("\n------ Product List ------\n");
    for (int i = 0; i < totalProducts; i++) {
        printProduct(products[i]);
    }
    printf("---------------------------\n");
}

int findProductIndexById(struct Product *products, int totalProducts, int id) {
    for (int i = 0; i < totalProducts; i++) {
        if (products[i].prodId == id)
            return i;
    }
    return -1;
}

void addNewProduct(struct Product **products, int *totalProducts, int *capacity) {
    if (*totalProducts >= *capacity) {
        int newCap = (*capacity == 0) ? 1 : (*capacity * 2);
        struct Product *temp = realloc(*products, newCap * sizeof(struct Product));
        if (temp == NULL) {
            printf("Memory reallocation failed!\n");
            return;
        }
        *products = temp;
        *capacity = newCap;
    }

    struct Product newProd;

    printf("\nEnter details for new Product:\n");
    printf("Product ID : ");
    scanf("%d", &newProd.prodId);
    clearInputBuffer();

    if (newProd.prodId < MIN_PROD_ID || newProd.prodId > MAX_PROD_ID) {
        printf("Invalid Product ID !!! Must be between 1 and 10000.\n");
        return;
    }

    if (findProductIndexById(*products, *totalProducts, newProd.prodId) != -1) {
        printf("Product ID already exists !!!\n");
        return;
    }

    while (1) {
        printf("Product Name : ");
        fgets(newProd.prodName, MAX_PROD_NAME, stdin);
        if (newProd.prodName[strlen(newProd.prodName) - 1] == '\n')
            newProd.prodName[strlen(newProd.prodName) - 1] = '\0';
        if (strlen(newProd.prodName) == 0) {
            printf("Product name cannot be empty!!!\n");
            continue;
        }
        if (strlen(newProd.prodName) > 50) {
            printf("Name too long !!! Max 50 chars Re-enter details.\n");
            return;
        }
        break;
    }

    printf("Product Price : ");
    scanf("%f", &newProd.prodPrice);

    if (newProd.prodPrice < MIN_PROD_PRICE || newProd.prodPrice > MAX_PROD_PRICE) {
        printf("Invalid price!!! Must be 0-100000 \n");
        clearInputBuffer();
        return;
    }

    printf("Product Quantity : ");
    scanf("%d", &newProd.prodQuantity);
    clearInputBuffer();

    if (newProd.prodQuantity < MIN_PROD_QUANTITY || newProd.prodQuantity > MAX_PROD_QUANTITY) {
        printf("Invalid quantity !!!\n");
        return;
    }

    (*products)[*totalProducts] = newProd;
    (*totalProducts)++;

    printf("Product added successfully !!!\n");
}

void updateQuantity(struct Product *products, int totalProducts) {
    int id;
    printf("Enter Product ID to update: ");
    scanf("%d", &id);
    clearInputBuffer();

    int index = findProductIndexById(products, totalProducts, id);
    if (index == -1) {
        printf("Product not found !!!\n");
        return;
    }

    int newQty;
    printf("Enter new quantity: ");
    scanf("%d", &newQty);
    clearInputBuffer();

    if (newQty < MIN_PROD_QUANTITY || newQty > MAX_PROD_QUANTITY) {
        printf("Invalid quantity !!!\n");
        return;
    }

    products[index].prodQuantity = newQty;
    printf("Quantity updated successfully !!!\n");
}

void searchProductByName(struct Product *products, int totalProducts) {
    char searchName[100];
    printf("Enter name to search: ");
    fgets(searchName, 100, stdin);
    if (searchName[strlen(searchName) - 1] == '\n')
        searchName[strlen(searchName) - 1] = '\0';

    int found = 0;
    for (int i = 0; i < totalProducts; i++) {
        if (strstr(products[i].prodName, searchName)) {
            printProduct(products[i]);
            found = 1;
        }
    }
    if (!found)
        printf("No product found with that name !!\n");
}

void searchByPriceRange(struct Product *products, int totalProducts) {
    float minPrice, maxPrice;
    printf("Enter minimum price: ");
    scanf("%f", &minPrice);
    printf("Enter maximum price: ");
    scanf("%f", &maxPrice);
    clearInputBuffer();

    if (minPrice > maxPrice) {
        printf("Invalid range !!!\n");
        return;
    }

    int found = 0;
    for (int i = 0; i < totalProducts; i++) {
        if (products[i].prodPrice >= minPrice && products[i].prodPrice <= maxPrice) {
            printProduct(products[i]);
            found = 1;
        }
    }
    if (!found)
        printf("No products found in that range !!!\n");
}

void deleteProduct(struct Product *products, int *totalProducts) {
    int id;
    printf("Enter Product ID to delete: ");
    scanf("%d", &id);
    clearInputBuffer();

    int index = findProductIndexById(products, *totalProducts, id);
    if (index == -1) {
        printf("Product not found !!!\n");
        return;
    }

    for (int i = index; i < *totalProducts - 1; i++) {
        products[i] = products[i + 1];
    }
    (*totalProducts)--;

    printf("Product deleted successfully !!!\n");
}

int main() {
    int initialProducts;
    printf("Enter initial number of Products : ");
    scanf("%d", &initialProducts);
    clearInputBuffer();

    if (initialProducts < MIN_PRODUCTS || initialProducts > MAX_PRODUCTS) {
        printf("Invalid number of products !!! Must be 0 - 100 \n");
        return 0;
    }

    struct Product *products = (struct Product *)calloc(initialProducts, sizeof(struct Product));
    
    if (products == NULL) {
        printf("Memory allocation failed !!! Try Again \n");
        return 0;
    }

    int totalProducts = 0;
    int capacity = initialProducts;

    for (int i = 0; i < initialProducts; i++) {
        printf("\nEnter details for Product %d:\n", i + 1);
        printf("Product ID : ");
        scanf("%d", &products[i].prodId);
        clearInputBuffer();

        if (products[i].prodId < MIN_PROD_ID || products[i].prodId > MAX_PROD_ID) {
            printf("Invalid Product ID !!! Must be 0 - 10000. Re-enter details.\n");
            i--;
            continue;
        }

        printf("Product Name : ");
        fgets(products[i].prodName, MAX_PROD_NAME, stdin);

        if (products[i].prodName[strlen(products[i].prodName) - 1] == '\n')
            products[i].prodName[strlen(products[i].prodName) - 1] = '\0';

        if (strlen(products[i].prodName) > 50) {
            printf("Name too long!!! Max 50 chars Re-enter details.\n");
            i--;
            continue;
        }

        printf("Product Price : ");
        scanf("%f", &products[i].prodPrice);
        printf("Product Quantity : ");
        scanf("%d", &products[i].prodQuantity);
        clearInputBuffer();

        if (products[i].prodPrice < MIN_PROD_PRICE || products[i].prodPrice > MAX_PROD_PRICE ||
            products[i].prodQuantity < MIN_PROD_QUANTITY || products[i].prodQuantity > MAX_PROD_QUANTITY) {
            printf("Invalid price or quantity! Price Must be 0-100000 and Quantity 0-1000000 Re-enter details.\n");
            i--;
            continue;
        }

        totalProducts++;
    }

    int choice;
    while (1) {
        
        printf("\n========== INVENTORY MENU ==========\n");
        printf("1. Add New Product\n");
        printf("2. View All Products\n");
        printf("3. Update Quantity\n");
        printf("4. Search Product by Name\n");
        printf("5. Search Product by Price Range\n");
        printf("6. Delete Product\n");
        printf("7. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        clearInputBuffer();

        switch (choice) {
            case 1:
                addNewProduct(&products, &totalProducts, &capacity);
                break;
            case 2:
                viewAllProducts(products, totalProducts);
                break;
            case 3:
                updateQuantity(products, totalProducts);
                break;
            case 4:
                searchProductByName(products, totalProducts);
                break;
            case 5:
                searchByPriceRange(products, totalProducts);
                break;
            case 6:
                deleteProduct(products, &totalProducts);
                break;
            case 7:
                free(products);
                printf("Memory released successfully. Exiting the program...Thank You\n");
                return 0;
            default:
                printf("Invalid choice !!! Try again.\n");
        }
    }
}
