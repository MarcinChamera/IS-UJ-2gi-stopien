var expressValidator = require('express-validator');

function userValidation() {
 return [
   expressValidator.body('email')
   .isEmail()
   .withMessage('Must be a valid email address.'),
   expressValidator.body('password')
   .isLength({ min: 8 })
   .withMessage('Password must be at least 8 characters long.'),
   expressValidator.body('passwordConfirm')
   .custom((value, { req }) => {
     if (value === req.body.password) {
       return true;
     } else {
       return false;
     }
   })
  .withMessage('Those passwords do not match. Try again.')
 ];
};

module.exports = userValidation;