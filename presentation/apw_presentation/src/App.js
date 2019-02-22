import React, { Component } from 'react';
import { Link, Route, Switch } from 'react-router-dom';
import Home from './Routes/Home/Home';
import Secret from './Routes/Secret/Secret';
import Login from './Routes/Login/Login';
import withAuth from './withAuth';

export default class App extends React.Component {
  render() {
    return (
      <div>
        <ul>
          <li><Link to="/">Home</Link></li>
          <li><Link to="/secret">Secret</Link></li>
          <li><Link to="/login">Login</Link></li>
        </ul>

        <Switch>
          <Route path="/" exact component={Home} />
          <Route path="/secret" component={withAuth(Secret)} />
          <Route path="/login" component={Login} />
        </Switch>
      </div>
    );
  }
}
