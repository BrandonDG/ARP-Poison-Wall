import React, { Component } from 'react';
import { Link, Route, Switch } from 'react-router-dom';
import Home from './Routes/Home/Home';
import Secret from './Routes/Secret/Secret';
import Login from './Routes/Login/Login';
import withAuth from './withAuth';
import { Button, Navbar, Nav, NavDropdown, Form, FormControl } from 'react-bootstrap';

export default class App extends React.Component {
  render() {
    return (

      <div>
        <div>
          <Navbar bg="light" expand="lg">
            <Navbar.Brand><Link to="/">ARP Poison Wall</Link></Navbar.Brand>
            <Navbar.Toggle aria-controls="basic-navbar-nav" />
            <Navbar.Collapse id="basic-navbar-nav">
              <Nav className="mr-auto">
                <Nav.Link ><Link to="/">Home</Link></Nav.Link>
                <Nav.Link ><Link to="/secret">Secret</Link></Nav.Link>
                <Nav.Link ><Link to="/login">Login</Link></Nav.Link>
              </Nav>
            </Navbar.Collapse>
          </Navbar>
        </div>

        <Switch>
          <Route path="/" exact component={withAuth(Home)} />
          <Route path="/secret" component={withAuth(Secret)} />
          <Route path="/login" component={Login} />
        </Switch>
      </div>
    );
  }
}
