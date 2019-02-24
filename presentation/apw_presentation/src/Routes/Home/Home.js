import React, { Component } from 'react';
import { Table } from 'react-bootstrap';

function HostTableRow(props) {
  return (
    <tr>
      <td>{props.value}</td>
    </tr>
  );
}

function HostTable(props) {
  return (
    <Table striped bordered hover>
      <thead>
        <tr>
          <th>Host</th>
        </tr>
      </thead>
      <tbody>
        {
          props.value
        }
      </tbody>
    </Table>
  );
}

export default class Home extends Component {
  constructor() {
    super();
    //Set default message
    this.state = {
      message: 'Loading...',
      ips: []
    }
  }

  componentDidMount() {
    //GET message from server using fetch api
    fetch('api/home')
      .then(res => res.text())
      .then(res => {
        this.setState({message: "Welcome", ips: JSON.parse(res)});
        console.log(this.state.ips);
      });
  }

  render() {
    return (
      <div>
        <h1>Home</h1>
        <p>{this.state.message}</p>
        <HostTable value={this.state.message} >
        </HostTable>
      </div>
    );
  }
}
