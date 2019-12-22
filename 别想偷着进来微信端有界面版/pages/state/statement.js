var myCharts = require("../../utils/chart.js")
const devicesId = "577208800" 
const api_key = "DQ4QOInXKfFaF=TPku5=tuCeFj0="
Page({
  data: {},
  onPullDownRefresh: function () {
    wx.showLoading({
      title: "正在获取"
    })
    this.getDatapoints().then(datapoints => {
      this.update(datapoints)
      wx.hideLoading()
    }).catch((error) => {
      wx.hideLoading()
      console.error(error)
    })
  },
  onLoad: function () {
    console.log(`your deviceId: ${devicesId}, apiKey: ${api_key}`)

    //每隔6s自动获取一次数据进行更新
    const timer = setInterval(() => {
      this.getDatapoints().then(datapoints => {
        this.update(datapoints)
      })
    }, 3000)

    wx.showLoading({
      title: '加载中'
    })

    this.getDatapoints().then((datapoints) => {
      wx.hideLoading()
      this.firstDraw(datapoints)
    }).catch((err) => {
      wx.hideLoading()
      console.error(err)
      clearInterval(timer) //首次渲染发生错误时禁止自动刷新
    })
  },
  getDatapoints: function () {
    return new Promise((resolve, reject) => {
      wx.request({
        url: `https://api.heclouds.com/devices/${devicesId}/datapoints?datastream_id=State`,
        header: {
          'content-type': 'application/json',
          'api-key': api_key
        },
        success: (res) => {
          const status = res.statusCode
          const response = res.data
          if (status !== 200) { // 返回状态码不为200时将Promise置为reject状态
            reject(res.data)
            return;
          }
          if (response.errno !== 0) { //errno不为零说明可能参数有误, 将Promise置为reject
            reject(response.error)
            return;
          }
          
          if (response.data.datastreams.length === 0) {
            reject("当前设备无数据, 请先运行硬件实验")
          }

          //程序可以运行到这里说明请求成功, 将Promise置为resolve状态
          resolve({
            
            state: response.data.datastreams[0].datapoints.reverse()
          })
      
        },
        fail: (err) => {
          reject(err)
        }
      })
    })
  },
  update: function (datapoints) {
    const wheatherData = this.convert(datapoints);

    this.lineChart_state.updateData({
      categories: wheatherData.categories,
      series: [{
        name: 'state',
        data: wheatherData.state,
        format: (val, name) => val.toFixed(2)
      }],
    })

  },
  convert: function (datapoints) {
    var categories = [];
    var state = [];

    var length = datapoints.state.length
    for (var i = 0; i < length; i++) {
      categories.push(datapoints.state[i].at.slice(5, 19));
      state.push(datapoints.state[i].value);
    }
    return {
      categories: categories,
     state:state
    }
  },
  firstDraw: function (datapoints) {

    //得到屏幕宽度
    var windowWidth = 320;
    try {
      var res = wx.getSystemInfoSync();
      windowWidth = res.windowWidth;
    } catch (e) {
      console.error('getSystemInfoSync failed!');
    }

    var wheatherData = this.convert(datapoints);

    //新建状态图表
    this.lineChart_sta = new myCharts({
      canvasId: 'state',
      type: 'line',
      categories: wheatherData.categories,
      animation: false,
      background: '#f5f5f5',
      series: [{
        name: 'state',
        data: wheatherData.state,
        format: function (val, name) {
          return val.toFixed(2);
        }
      }],
      xAxis: {
        disableGrid: true
      },
      yAxis: {
        title: 'state（0means无人1means有人）',
        format: function (val) {
          return val.toFixed(2);
        }
      },
      width: windowWidth,
      height: 200,
      dataLabel: false,
      dataPointShape: true,
      extra: {
        lineStyle: 'curve'
      }
    });  
    }
})