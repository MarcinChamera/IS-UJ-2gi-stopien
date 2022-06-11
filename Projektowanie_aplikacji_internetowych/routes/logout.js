var express = require('express');
var router = express.Router();
var { currentUser } = require('../appMiddlewares.js');

router.post('/', currentUser, function(req, res) {
    req.session.destroy((error) => {
        if (error) throw error;
        console.log('User logout.');
        res.redirect('/');
      });
});

module.exports = router;
