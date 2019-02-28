import React, { Component } from 'react';
import { Link, Route, Switch } from 'react-router-dom';
import Home from './Routes/Home/Home';
import Host from './Routes/Host/Host';
import Configuration from './Routes/Configuration/Configuration';
import Login from './Routes/Login/Login';
import withAuth from './withAuth';
import { Navbar, Nav } from 'react-bootstrap';

export default class App extends React.Component {
  constructor(props) {
    super(props);
    this.state = {
      selectedip: "192.168.0.1"
    };
  }

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
                <Nav.Link ><Link to="/Configuration">Configuration</Link></Nav.Link>
                <Nav.Link ><Link to="/login">Login</Link></Nav.Link>
              </Nav>
            </Navbar.Collapse>
          </Navbar>
        </div>

        <Switch>
          <Route path="/" exact component={withAuth(Home)} />
          <Route path="/Host/:ip" component={withAuth(Host)} />
          <Route path="/Configuration" component={withAuth(Configuration)} />
          <Route path="/login" component={Login} />
        </Switch>
      </div>
    );
  }
}
