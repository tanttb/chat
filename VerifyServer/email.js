
const nodemailer = require("nodemailer")
const config_module = require("./config")


let transport = nodemailer.createTransport({
   host: 'smtp.qq.com',
   port: 465,
   secure: true,
   auth:{
      user : config_module.email_user,
      pass : config_module.email_pass
   }
});

/**
 * 发送邮件的函数
 * @param {*} mailOptions_ 发送邮件的参数
 * @returns 
 */
function SendMail(mailOptions_){
    return new Promise(function(resolve, reject){
        transport.sendMail(mailOptions_, function(error, info){
            if (error) {
                console.log(error);
                reject(error);
            } else {
                console.log('邮件已成功发送：' + info.response);
                resolve(info.response);
            }
        });
    })
}
module.exports.SendMail = SendMail