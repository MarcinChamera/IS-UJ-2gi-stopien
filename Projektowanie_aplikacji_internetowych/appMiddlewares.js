function isNotAuth (request, response, next) {
    if (request.session.isAuth) {
      next();
    } else {
      response.status(401).render('401');
    }
   };
   
function isAuth (request, response, next) {
    if (request.session.isAuth) {
        response.redirect('/');
    } else {
        next();
    }
};

function currentUser (request, response, next) {
    if (request.session.userEmail) {
        response.locals.userEmail = request.session.userEmail;
        next();
    } else {
        response.locals.userEmail = null;
        next();
    }
};

module.exports = {isNotAuth, isAuth, currentUser};