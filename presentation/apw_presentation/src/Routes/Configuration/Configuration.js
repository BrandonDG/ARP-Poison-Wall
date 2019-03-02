import React, { Component } from 'react';
import { Form, Button, Row, Col } from 'react-bootstrap';

export default class Configuration extends Component {
  constructor() {
    super();
    //Set default message
    this.state = {
      message: 'Loading...',
      threshold: 'Strict',
      router_mac: '',
      router_ip: '',
      password: ''
    };
  }

  componentDidMount() {
    //GET message from server using fetch api
    fetch('/api/secret')
      .then(res => res.text())
      .then(res => this.setState({message: res}));
  }

  handleChange = e => {
    this.setState({ [e.target.name]: e.target.value });
  };

  handleSubmit = e => {
    fetch('/api/configuration', {
      method: 'POST',
      body: JSON.stringify(this.state),
      headers: {
        'Content-Type': 'application/json'
      }
    })
    .then(res => {
      if (res.status === 200) {
        this.props.history.push('/');
      } else {
        const error = new Error(res.error);
        throw error;
      }
    })
    .catch(err => {
      console.error(err);
      alert('Error logging in please try again');
    });
    e.preventDefault();
  };

  render() {
    return (
      <div>
        <h1>Configuration</h1>
        <p>{this.state.message}</p>
        <Form onSubmit={this.handleSubmit}>
          <Row>
            <Col>
              <Form.Group controlId="formBasicRouterIp">
                <Form.Label>Router IP</Form.Label>
                <Form.Control type="routerip" placeholder="Router IP"
                            name="router_ip" onChange={this.handleChange} />
              </Form.Group>
            </Col>
            <Col>
              <Form.Group controlId="formBasicRouterMac">
                <Form.Label>Router MAC</Form.Label>
                <Form.Control type="routermac" placeholder="Router MAC"
                            name="router_mac" onChange={this.handleChange} />
              </Form.Group>
            </Col>
          </Row>
          <Row>
            <Form.Group as={Col} controlId="formPassword">
              <Form.Label>Password</Form.Label>
              <Form.Control type="password" placeholder="Password"
                          name="password" onChange={this.handleChange} />
            </Form.Group>
          </Row>
          <Row>
          <Form.Group as={Col} controlId="formThreshold">
            <Form.Label>Threshold</Form.Label>
            <Form.Control as="select" name="threshold" onChange={this.handleChange}>
              <option>Strict</option>
              <option>Normal</option>
              <option>Lenient</option>
            </Form.Control>
          </Form.Group>
          </Row>
          <Button variant="primary" type="submit">
            Submit
          </Button>
        </Form>
      </div>
    );
  }

}
