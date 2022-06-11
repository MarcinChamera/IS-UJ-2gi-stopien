var appDatabase = require('./database/db.js');

function findUser (email, callback) {
  const sql = "SELECT * FROM users WHERE Email = ?";
  appDatabase.get(sql, [email], (error, row) => {
    if (error) {
      console.log(error.message);
      callback(error.message);
    } else {
      callback(row);
    };
  })
};

function registerUser (email, password, callback) {
  const sql = 'INSERT INTO users (Email, Password) VALUES (?, ?)';
  appDatabase.run(sql, [email, password], (error, row) => {
    if (error) {
      callback(error.message);
    } else {
      const successMessage = 'User registration successful.';
      callback(successMessage);
    };
  });
};

function showExploreArticles(email, callback) {
  var data = {};
  var sql = {};
  if (email == undefined) {
    sql = `SELECT Email, Header, Content
           FROM articles a
           LEFT JOIN users u
           ON a.UserID = u.UserID`;
  } else {
    sql = `SELECT Email, Header, Content
           FROM articles a
           LEFT JOIN users u
           ON a.UserID = u.UserID
           WHERE Email != ?`;
  }      
  appDatabase.all(sql, email, (error, rows) => {
    if (error) {
      console.log(error.message);
      callback(error.message);
    } else {
      data.rows = rows;
      callback(data);
    };
  })
}

function showUserArticles(email, callback) {
  var data = {};
  const sql = `SELECT ArticleID, Header, Content
               FROM articles a
               LEFT JOIN users u
               ON a.UserID = u.UserID
               WHERE Email = ?`;
  appDatabase.all(sql, [email], (error, rows) => {
    if (error) {
      console.log(error.message);
      callback(error.message);
    } else {
      data.rows = rows;
      callback(data);
    };
  })
}

function saveNewArticle (header, content, email, callback) {
  findUser(email, (result) => {
    const sql = 'INSERT INTO articles (Header, Content, UserID) VALUES (?, ?, ?)';
    appDatabase.run(sql, [header, content, result.UserID], (error, row) => {
      if (error) {
        callback(error.message);
      } else {
        const successMessage = 'New post added successfully.';
        callback(successMessage);
      };
    });
  });
};

function findArticleById(id, callback) {
  const sql = 'SELECT ArticleID, Header, Content FROM articles WHERE ArticleID = ?';
  appDatabase.get(sql, [id], (error, row) => {
    if (error) {
      console.log(error.message);
      callback(error.message);
    } else {
      callback(row);
    };
  })
};

function updateArticle(id, title, content, callback) {
  const sql = 'UPDATE articles SET Header = ?, Content = ? WHERE ArticleID = ?';
  appDatabase.run(sql, [title, content, id], (error, row) => {
    if (error) {
      console.log(error.message);
      callback(error.message);
    } else {
      callback('Article updated successfully');
    }
  });
}

function deleteArticle (id) {
  const sql = 'DELETE FROM articles where ArticleID = ?';
  appDatabase.run(sql, id, (error, row) => {
    if (error) {
      console.log(error.message);
    }
  });};

module.exports = {
  findUser,
  registerUser,
  showExploreArticles,
  showUserArticles,
  saveNewArticle,
  findArticleById,
  updateArticle,
  deleteArticle
};