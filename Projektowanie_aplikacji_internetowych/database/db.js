var sqlite3 = require('sqlite3').verbose();

database = new sqlite3.Database('database/db.db', sqlite3.OPEN_READWRITE, error => {
 if (error) {
   console.error(error.message);
 } else {
  console.log('Successfully connected to the database');
 }
});

module.exports = database;