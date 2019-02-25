import React, { Component } from 'react';

export default class Host extends Component {
  constructor(props) {
    super(props);
    //Set default message
    this.state = {
      message: 'Loading...',
    };
  }

  /*
  fetch('/api/authenticate', {
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
  */

  /*
  componentDidMount() {
    //GET message from server using fetch api
    fetch('/api/host', {
      method: 'GET',
      body: JSON.stringify(this.props.match.params.ip),
      headers: {
        'Content-Type': 'application/json'
      },
      mode: 'cors'
    })
      .then(res => res.text())
      .then(res => this.setState({message: res}));
  }
  */

  componentDidMount() {
    //GET message from server using fetch api
    fetch('/api/host', {
      method: 'GET',
      headers: {
        'selectedip': this.props.match.params.ip
      }
    })
      .then(res => res.text())
      .then(res => {
        var json_res = JSON.parse(res);
        console.log(res);
        this.setState({message: "This is host page"});
      });
  }

  render() {
    return (
      <div>
        <h1>Host</h1>
        <p>{this.state.message}</p>
        <p>{this.props.match.params.ip}</p>
      </div>
    );
  }

}
