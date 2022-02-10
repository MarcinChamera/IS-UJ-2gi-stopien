# Prerequirements:
* Start running PostgreSQL database:
    
    ```
    sudo service postgresql start
    ```

* Start lapis server from root directory:
   
   ``` 
    lapis server
   ```

# How to test:
1. Install REST Client extension for Visual Studio Code
2. Use "Send request" for chosen request from api_test.http

# Other:
* "categories" table was created using query:
    
    ```
    CREATE TABLE categories (
        id SERIAL PRIMARY KEY,
        name VARCHAR(60) NOT NULL
    );
    ```
* "products" table was created using query:
    
    ```
    CREATE TABLE products (
        id SERIAL PRIMARY KEY,
        name VARCHAR(100) NOT NULL,
        category_id INT,
        CONSTRAINT fk_category
        FOREIGN KEY(category_id)
        REFERENCES categories(id)
    );
    ```

