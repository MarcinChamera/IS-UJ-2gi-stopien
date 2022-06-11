var createError = require('http-errors');
var express = require('express');
var path = require('path');
var logger = require('morgan');
var connectSqlite = require('connect-sqlite3');
var session = require('express-session');
var methodOverride = require('method-override');

var indexRouter = require('./routes/index');
var registerRouter = require('./routes/register');
var loginRouter = require('./routes/login');
var logoutRouter = require('./routes/logout');
var userArticlesRouter = require('./routes/userArticles');

var app = express();

app.set('view engine', 'pug');
app.set('views', path.join(__dirname, 'views'));

app.use(logger('dev'));
app.use(express.json());
app.use(express.urlencoded({ extended: false }));
app.use(express.static(path.join(__dirname, 'public')));
app.use(methodOverride('_method'));

const SQLiteStore = connectSqlite(session);

app.use(session({
  store: new SQLiteStore({
    dir: './database/',
    db: 'db.db',
    table: 'sessions'
  }),
  secret: 'j2b319o48hykbj1923g',
  resave: false,
  saveUninitialized: false,
  cookie: {
    maxAge: 1000 * 60 * 10,
    sameSite: true
  }
}));

app.use('/', indexRouter);
app.use('/register', registerRouter);
app.use('/login', loginRouter);
app.use('/logout', logoutRouter);
app.use('/userArticles', userArticlesRouter);

module.exports = app;