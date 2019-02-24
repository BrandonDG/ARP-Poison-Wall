import React, { Component } from 'react';
import { Table } from 'react-bootstrap';
import BootstrapTable from 'react-bootstrap-table-next';

/*
function HostTableRow(props) {
  const rows = props.value.map((ip) =>
    <tr>
      <td>ip.ip</td>
    </tr>
  );
  return (
    {rows}
  );
}

function HostTable(props) {
  console.log(props.value);
  return (
    <Table striped bordered hover>
      <thead>
        <tr>
          <th>Host</th>
        </tr>
      </thead>
      <tbody>
        <HostTableRow value={props.value} />
      </tbody>
    </Table>
  );
} */

export default class Home extends Component {
  constructor() {
    super();
    //Set default message
    this.state = {
      message: 'Loading...',
      ips: [],
      columns: [{
        dataField: 'ip',
        text: "Host",
        events: {
          onClick: (e, column, columnIndex, row, rowIndex) => {
            console.log(row);
          }
        }
      }]
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
        <BootstrapTable keyField="ip" data={ this.state.ips } columns = { this.state.columns } />
      </div>
    );
  }
}
