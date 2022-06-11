var express = require('express');
var router = express.Router();
var { currentUser, isNotAuth } = require('../appMiddlewares.js');
var { showUserArticles, saveNewArticle, findArticleById, updateArticle, deleteArticle } = require("../appModels.js");

router.get('/', isNotAuth, currentUser, function(req, res, next) {
  showUserArticles(req.session.userEmail, (result) => {
    if (result == {}) {
      const confilctError = 'No articles available';
      console.log(confilctError);
      res.render('userArticles', { title: 'Blog App', 'conflictError' : confilctError });
    }
    else{
      console.log('Articles found');
      res.render('userArticles', { 'articles' : result });
    }
  });
});

router.get('/new', isNotAuth, currentUser, function(req, res, next) {
  res.render('userArticlesNew', {headerTitle: 'New article'});
});

router.post('/new', isNotAuth, currentUser, function(req, res) {
  const { title, content } = req.body;
  saveNewArticle(title, content, req.session.userEmail, (result) => {
      console.log(result);
      res.redirect('/userArticles');
  });
});

router.get('/edit/:id', isNotAuth, currentUser, function(req, res, next) {
  console.log(req.params.id);
  findArticleById(req.params.id, (result) => {
    res.render('userArticlesEdit', {headerTitle : 'Edit article', article: result});
  });
});

router.post('/edit/:id', isNotAuth, currentUser, function(req, res, next) {
  const { title, content } = req.body;
  updateArticle(req.params.id, title, content, (result) => {
      console.log(result);
      res.redirect('/userArticles');
  });
});

router.delete('/:id', isNotAuth, currentUser, (req, res) => {
  deleteArticle(req.params.id);
  res.redirect('/userArticles');
});

module.exports = router;