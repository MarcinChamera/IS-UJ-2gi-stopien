var express = require('express');
var router = express.Router();
var { currentUser } = require('../appMiddlewares.js');
var { showExploreArticles } = require("../appModels.js");

router.get('/', currentUser, function(req, res) {
  showExploreArticles(req.session.userEmail, (result) => {
    if (result == {}) {
      const confilctError = 'No articles available';
      console.log(confilctError);
      res.render('index', { title: 'Blog App', 'conflictError' : confilctError });
    }
    else{
      console.log('Articles found');
      console.log(result);
      res.render('index', {'articles' : result });
    }
  });
});

module.exports = router;
