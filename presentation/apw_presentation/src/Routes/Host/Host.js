import React, { Component } from 'react';
import BootstrapTable from 'react-bootstrap-table-next';
import paginationFactory from 'react-bootstrap-table2-paginator';

export default class Host extends Component {
  constructor(props) {
    super(props);
    //Set default message
    this.state = {
      message: 'Loading...',
      host_info: {
        alerts: [],
        logs: [],
        status: ""
      },
      alert_columns: [{
        dataField: 'alertid',
        text: 'Alert ID'
      }, {
        dataField: 'from_a',
        text: 'Attacker'
      }, {
        dataField: 'to_a',
        text: 'Victim'
      }, {
        dataField: 'start_t',
        text: 'Start Time'
      }, {
        dataField: 'end_t',
        text: 'End Time'
      }, {
        dataField: 'status',
        text: 'Status'
      }],
      log_columns: [{
        dataField: 'logid',
        text: 'Log ID'
      }, {
        dataField: 'from_a',
        text: 'Attacker'
      }, {
        dataField: 'to_a',
        text: 'Victim'
      }, {
        dataField: 'time_s',
        text: 'Timestamp'
      }]
    };
  }

  componentDidMount() {
    fetch('/api/host', {
      method: 'GET',
      headers: {
        'selectedip': this.props.match.params.ip
      }
    })
      .then(res => res.text())
      .then(res => {
        this.setState({
          message: "This is host page",
          host_info: JSON.parse(res)
        });
      });
  }

  render() {
    console.log(this.state.host_info.alerts);
    console.log(this.state.host_info.logs);
    return (
      <div>
        <h1>Host</h1>
        <h2>Host IP: {this.props.match.params.ip}</h2>
        <h2>Status:  {this.state.host_info.status}</h2>
        <h2>Alerts</h2>
        <BootstrapTable keyField="alertid" data={ this.state.host_info.alerts } columns = { this.state.alert_columns } />
        <h2>Logs</h2>
        <BootstrapTable keyField="logid" data={ this.state.host_info.logs } columns = { this.state.log_columns } />
      </div>
    );
  }

}
