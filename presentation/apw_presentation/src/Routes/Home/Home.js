import React, { Component } from 'react';
import BootstrapTable from 'react-bootstrap-table-next';

export default class Home extends Component {
  constructor(props) {
    super(props);
    //Set default message
    this.state = {
      message: 'Loading...',
      ips: [],
      columns: [{
        dataField: 'ip',
        text: "Host",
        events: {
          onClick: (e, column, columnIndex, row, rowIndex) => {
            console.log(row.ip);
            this.props.history.push('/Host/' + row.ip);
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
