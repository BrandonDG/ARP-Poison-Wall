import React, { Component } from 'react';

export default class Host extends Component {
  constructor(props) {
    super(props);
    //Set default message
    this.state = {
      message: 'Loading...',
    };
  }

  componentDidMount() {
    //GET message from server using fetch api
    fetch('/api/host')
      .then(res => res.text())
      .then(res => this.setState({message: res}));
  }

  render() {
    return (
      <div>
        <h1>Host</h1>
        <p>{this.props.match.params.ip}</p>
      </div>
    );
  }

}
