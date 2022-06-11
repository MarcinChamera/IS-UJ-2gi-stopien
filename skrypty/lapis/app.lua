-- app.lua
local lapis = require("lapis")
local util = require("lapis.util")
local config = require("lapis.config").get()

local Model = require("lapis.db.model").Model
local Categories = Model:extend("categories")
local Products = Model:extend("products")

local app = lapis.Application()

app:get("/", function()
  return "Welcome to Lapis " .. require("lapis.version")
end)

app:get("/categories", function(self)
  return {
    json = Categories:select()
    
  }
end)

app:get("/categories/:category_name", function(self)
  return {
    json = Categories:find({ name = self.params.category_name })
  }
end)

app:post("/categories/:category_name", function(self)
  local category = Categories:create({
    id = db.raw("(nextval('categories_id_seq'))"),
    name = self.params.category_name
  })
  return {
    json = category
  }
end)

app:put("/categories/:old_category_name/:new_category_name", function(self)
  local category = Categories:find({ name = self.params.old_category_name })
  category:update({
    name = self.params.new_category_name
  })
  return {
    json = category
  }
end)

app:delete("/categories/:category_id", function(self)
  local category = Categories:find({ id = self.params.category_id })
  if category then
    result = category:delete()
    if result then
      return {
        json = category
      }
    end
  end
end)

app:get("/products", function(self)
  return {
    json = Products:select()
  }
end)

app:get("/products/:product_id", function(self)
  local product = Products:find({ 
    id = self.params.product_id
  })
  return {
    json = product
  }
end)

app:post("/products/:product_name/:category_id", function(self)
  local product = Products:create({
    name = self.params.product_name,
    category_id = self.params.category_id
  })
  return {
    json = product
  }
end)

app:put("/products/:product_id/:new_product_name", function(self)
  local product = Products:find({ id = self.params.product_id })
  product:update({
    name = self.params.new_product_name
  })
  return {
    json = product
  }
end)

app:delete("/products/:product_id", function(self)
  local product = Products:find({ id = self.params.product_id })
  if product then 
    result = product:delete()
    if result then
      return {
        json = product
      }
    end
  end
end)

return app