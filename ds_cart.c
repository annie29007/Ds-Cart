#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define USER_FILE "users.txt"
#define BILL_FILE "bills.txt"
#define ADMIN_PASS "admin123"

//defining the structure Product
typedef struct Product
{
    int id;
    char name[64];
    char category[32];
    double price;
    int stock;
    struct Product *prev, *next;
} Product;

Product *product_head = NULL;

//function to create product
Product *create_product(int id, const char *name, const char *cat, double price, int stock)
{
    Product *p = (Product *)malloc(sizeof(Product));
    p->id = id;
    strcpy(p->name, name);
    strcpy(p->category, cat);
    p->price = price;
    p->stock = stock;
    p->prev = p->next = NULL;
    return p;
}

//Adds a new product node at the end of the doubly linked list.
void append_product(Product *node)
{
    if (!product_head)
    {
        product_head = node;
        return;
    }
    Product *cur = product_head;
    while (cur->next)
        cur = cur->next;
    cur->next = node;
    node->prev = cur;
}

//to find products by their id
Product *find_product_by_id(int id)
{
    Product *cur = product_head;
    while (cur)
    {
        if (cur->id == id)
            return cur;
        cur = cur->next;
    }
    return NULL;
}

//to display the products(Name,Id,Category,Price,stock)
void display_products()
{
    printf("\n%-5s %-15s %-12s %-10s %-8s\n", "ID", "Name", "Category", "Price", "Stock");
    printf("-------------------------------------------------------------\n");
    Product *cur = product_head;
    while (cur)
    {
        printf("%-5d %-15s %-12s %-10.2lf %-8d\n",
               cur->id, cur->name, cur->category, cur->price, cur->stock);
        cur = cur->next;
    }
}

// cart item node holding a pointer to a product, its quantity, and a link to the next item for implementing a stack 
typedef struct CartItem
{
    Product *product;
    int qty;
    struct CartItem *next;
} CartItem;

CartItem *cart_top = NULL;

//to push items into the cart
void push_cart(Product *p, int qty)
{
    CartItem *ci = (CartItem *)malloc(sizeof(CartItem));
    ci->product = p;
    ci->qty = qty;
    ci->next = cart_top;
    cart_top = ci;
}

//to remove items from the cart
int pop_cart()
{
    if (!cart_top)
        return 0;
    CartItem *t = cart_top;
    cart_top = cart_top->next;
    free(t);
    return 1;
}

//to display the items present in the cart
void display_cart()
{
    if (!cart_top)
    {
        printf("\nCart is empty.\n");
        return;
    }
    printf("\nYour Cart:\n");
    printf("%-5s %-15s %-8s %-10s\n", "ID", "Name", "Qty", "Price");
    printf("--------------------------------------\n");
    CartItem *cur = cart_top;
    while (cur)
    {
        printf("%-5d %-15s %-8d %-10.2lf\n",
               cur->product->id, cur->product->name, cur->qty, cur->product->price);
        cur = cur->next;
    }
}

//clear cart
void clear_cart()
{
    while (cart_top)
        pop_cart();
}

//FIFO queue for checkout, where each node stores a product, quantity, and a pointer to the next item, and the queue tracks the front and rear nodes.
typedef struct QNode
{
    Product *product;
    int qty;
    struct QNode *next;
} QNode;

typedef struct Queue
{
    QNode *front, *rear;
} Queue;

void init_queue(Queue *q) { q->front = q->rear = NULL; }

void enqueue(Queue *q, Product *p, int qty)
{
    QNode *n = (QNode *)malloc(sizeof(QNode));
    n->product = p;
    n->qty = qty;
    n->next = NULL;
    if (!q->rear)
    {
        q->front = q->rear = n;
        return;
    }
    q->rear->next = n;
    q->rear = n;
}

int dequeue(Queue *q, Product **p, int *qty)
{
    if (!q->front)
        return 0;
    QNode *t = q->front;
    *p = t->product;
    *qty = t->qty;
    q->front = q->front->next;
    if (!q->front)
        q->rear = NULL;
    free(t);
    return 1;
}

void cart_to_queue(Queue *q)
{
    init_queue(q);
    CartItem *cur = cart_top;
    int n = 0;
    while (cur)
    {
        n++;
        cur = cur->next;
    }
    if (n == 0)
        return;
    CartItem **arr = malloc(sizeof(CartItem *) * n);
    cur = cart_top;
    for (int i = 0; i < n; i++)
    {
        arr[i] = cur;
        cur = cur->next;
    }
    for (int i = n - 1; i >= 0; i--)
        enqueue(q, arr[i]->product, arr[i]->qty);
    free(arr);
}

/* -------------------- File Handling -------------------- */
int registerUser(const char *u, const char *p)
{
    FILE *f = fopen(USER_FILE, "a");
    if (!f)
        return 0;
    fprintf(f, "%s %s\n", u, p);
    fclose(f);
    return 1;
}
//user login
int loginUser(const char *u, const char *p)
{
    FILE *f = fopen(USER_FILE, "r");
    if (!f)
        return 0;
    char uu[64], pp[64];
    int ok = 0;
    while (fscanf(f, "%63s %63s", uu, pp) == 2)
    {
        if (strcmp(uu, u) == 0 && strcmp(pp, p) == 0)
        {
            ok = 1;
            break;
        }
    }
    fclose(f);
    return ok;
}

//billing
void save_bill(const char *user, double total, Queue *q)
{
    FILE *f = fopen(BILL_FILE, "a");
    if (!f)
        return;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(f, "\nUSER:%s DATE:%04d-%02d-%02d_%02d:%02d:%02d TOTAL:%.2lf\n",
            user, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec, total);
    QNode *cur = q->front;
    while (cur)
    {
        fprintf(f, "  %d %s %d %.2lf\n",
                cur->product->id, cur->product->name, cur->qty, cur->product->price);
        cur = cur->next;
    }
    fprintf(f, "------------------------------------\n");
    fclose(f);
}

void input_str(const char *prompt, char *buf, int size)
{
    printf("%s", prompt);
    if (fgets(buf, size, stdin))
    {
        buf[strcspn(buf, "\n")] = 0;
    }
    else
        buf[0] = 0;
}

int input_int(const char *prompt)
{
    char buf[64];
    input_str(prompt, buf, sizeof(buf));
    return atoi(buf);
}

//sample products
void load_sample_products()
{
    append_product(create_product(101, "Pen", "Stationery", 10.0, 100));
    append_product(create_product(102, "Notebook", "Stationery", 50.0, 80));
    append_product(create_product(103, "WaterBottle", "Utility", 199.0, 50));
    append_product(create_product(104, "Earphones", "Electronics", 399.0, 25));
    append_product(create_product(105, "Mouse", "Electronics", 799.0, 30));
}

//checkout-will show the total bill&items present
void process_checkout(const char *user)
{
    if (!cart_top)
    {
        printf("\nCart empty! Add items first.\n");
        return;
    }
    Queue q;
    cart_to_queue(&q);
    double total = 0;
    printf("\n--- Checkout ---\n");
    printf("%-15s %-8s %-10s %-10s\n", "Item", "Qty", "Price", "LineTotal");
    printf("----------------------------------------------\n");
    Product *p;
    int qty;
    while (dequeue(&q, &p, &qty))
    {
        if (p->stock < qty)
        {
            printf("%s: Not enough stock!\n", p->name);
            continue;
        }
        double lt = p->price * qty;
        printf("%-15s %-8d %-10.2lf %-10.2lf\n", p->name, qty, p->price, lt);
        total += lt;
        p->stock -= qty;
    }
    printf("----------------------------------------------\nTotal: %.2lf\n", total);
    save_bill(user, total, &q);
    clear_cart();
    printf("Bill saved. Thank you for shopping with DS Cart!\n");
}

//user menu-what the user sees 
void user_menu(const char *username)
{
    while (1)
    {
        printf("\n=== User Menu (%s) ===\n", username);
        printf("1. View Products\n2. Add to Cart\n3. Remove Last from Cart\n4. View Cart\n5. Checkout\n6. Logout\nChoose: ");
        char ch[8];
        if (!fgets(ch, sizeof(ch), stdin))
            break;
        int c = atoi(ch);
        if (c == 1)
            display_products();
        else if (c == 2)
        {
            int id = input_int("Enter product ID: ");
            Product *p = find_product_by_id(id);
            if (!p)
            {
                printf("No such product.\n");
                continue;
            }
            int qty = input_int("Enter quantity: ");
            if (qty > p->stock)
            {
                printf("Only %d left in stock.\n", p->stock);
                continue;
            }
            push_cart(p, qty);
            printf("Added %d x %s to cart.\n", qty, p->name);
        }
        else if (c == 3)
        {
            if (pop_cart())
                printf("Removed last item from cart.\n");
            else
                printf("Cart empty.\n");
        }
        else if (c == 4)
            display_cart();
        else if (c == 5)
            process_checkout(username);
        else if (c == 6)
            break;
        else
            printf("Invalid choice.\n");
    }
}

//admin's menu-what the admin sees & what operations it can perform
void admin_menu()
{
    char pass[64];
    input_str("Enter admin password: ", pass, sizeof(pass));
    if (strcmp(pass, ADMIN_PASS) != 0)
    {
        printf("Wrong password!\n");
        return;
    }
    while (1)
    {
        printf("\n=== Admin Menu ===\n");
        printf("1. View Products\n2. Add Product\n3. Delete Product\n4. Update Stock\n5. Back\nChoose: ");
        char buf[8];
        if (!fgets(buf, sizeof(buf), stdin))
            break;
        int ch = atoi(buf);
        if (ch == 1)
            display_products();
        else if (ch == 2)
        {
            int id = input_int("New ID: ");
            if (find_product_by_id(id))
            {
                printf("ID already exists.\n");
                continue;
            }
            char name[64], cat[32];
            input_str("Name: ", name, sizeof(name));
            input_str("Category: ", cat, sizeof(cat));
            char pbuf[32];
            input_str("Price: ", pbuf, sizeof(pbuf));
            double price = atof(pbuf);
            int stock = input_int("Stock: ");
            append_product(create_product(id, name, cat, price, stock));
            printf("Product added successfully!\n");
        }
        else if (ch == 3)
        {
            int id = input_int("ID to delete: ");
            Product *p = find_product_by_id(id);
            if (!p)
            {
                printf("Not found.\n");
                continue;
            }
            if (p->prev)
                p->prev->next = p->next;
            else
                product_head = p->next;
            if (p->next)
                p->next->prev = p->prev;
            free(p);
            printf("Deleted.\n");
        }
        else if (ch == 4)
        {
            int id = input_int("ID to update stock: ");
            Product *p = find_product_by_id(id);
            if (!p)
            {
                printf("Not found.\n");
                continue;
            }
            int s = input_int("New stock: ");
            p->stock = s;
            printf("Stock updated.\n");
        }
        else if (ch == 5)
            break;
        else
            printf("Invalid.\n");
    }
}

//main
int main()
{
    printf("===== Welcome to DS Cart =====\n");
    load_sample_products();

    while (1)
    {
        printf("\nMain Menu\n");
        printf("1. Register\n2. Login\n3. Admin\n4. Exit\nChoose: ");
        char buf[8];
        if (!fgets(buf, sizeof(buf), stdin))
            break;
        int ch = atoi(buf);
        if (ch == 1)
        {
            char u[64], p[64];
            input_str("Username: ", u, sizeof(u));
            input_str("Password: ", p, sizeof(p));
            if (registerUser(u, p))
                printf("Registered successfully!\n");
            else
                printf("Error registering user.\n");
        }
        else if (ch == 2)
        {
            char u[64], p[64];
            input_str("Username: ", u, sizeof(u));
            input_str("Password: ", p, sizeof(p));
            if (loginUser(u, p))
            {
                printf("Login success! Welcome, %s\n", u);
                user_menu(u);
            }
            else
            {
                printf("Login failed.\n");
            }
        }
        else if (ch == 3)
            admin_menu();
        else if (ch == 4)
        {
            printf("Goodbye!\n");
            break;
        }
        else
            printf("Invalid choice.\n");
    }
    return 0;
}

