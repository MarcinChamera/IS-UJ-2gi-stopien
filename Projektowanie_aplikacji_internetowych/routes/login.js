var express = require('express');
var router = express.Router();
var appModel = require('../appModels.js');
var bcrypt = require('bcrypt');
var { currentUser, isAuth } = require('../appMiddlewares.js');

router.get('/', isAuth, currentUser, function(req, res) {

    res.render('login');
});

router.post('/', isAuth, currentUser, function(req, res) {

    const { email, password } = req.body;
    const conflictError = 'User credentials are not valid.';
    appModel.findUser(email, (result) => {
    if (result != undefined) {
        const dbPassword = result.Password;
        bcrypt.compare(password, dbPassword, function(err, result) {
        if (result == true) {
            req.session.isAuth = true;
            req.session.userEmail = email;
            console.log('Login successful.')
            res.redirect('/');
        } else {
            console.log(conflictError);
            res.render('login', { email, password, conflictError });
        }
        });
    } else {
        console.log(conflictError);
        res.render('login', { email, password, conflictError });
    };
    }); 
});

module.exports = router;