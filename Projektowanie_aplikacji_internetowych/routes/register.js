var express = require('express');
var router = express.Router();
var { findUser, registerUser } = require("../appModels.js");
var expressValidator = require('express-validator');
var bcrypt = require('bcrypt');
var userValidation = require('../appValidator.js');
var { currentUser, isAuth } = require('../appMiddlewares.js');

router.get('/', isAuth, currentUser, function(req, res, next) {

    res.render('register');
});

const saltRounds = 10;

router.post('/', isAuth, currentUser, userValidation(), function(req, res) {
    const { email, password, passwordConfirm } = req.body;
    const validationErrors = expressValidator.validationResult(req);
    if (!validationErrors.isEmpty()) {
        return res.render('register', { email, password, passwordConfirm, validationErrors: validationErrors.mapped() });
    } else {
        findUser(email, (result) => {
            if (result != undefined) {
                const conflictError = 'User with this email already exists.';
                console.log(conflictError);
                res.render('register', { email, password, passwordConfirm, conflictError })
            } else {
                bcrypt.hash(password, saltRounds).then(function(hashedPassword) {
                    registerUser(email, hashedPassword, (result) => {
                        res.redirect('/login');
                    });
                });
            };
        });
    };
});

module.exports = router;