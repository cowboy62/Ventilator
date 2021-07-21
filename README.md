# Ventilator
Ventilator motion control


呼吸機步進馬達控制程式
使用AccelStepper庫
由三個可變電阻可以設定呼吸頻率,出氣量及壓力
可以參數設置的最大出氣量,呼吸時間比,
具有加減速功能,可參數設置加減速
簡單的程式設計及簡單的接口,可以快速改為通訊方式控制

在ARDUINO UNO 實現,

    應該也可以在MEGA 2560 使用(未測試)

在TB6560模組上實現,

    應該也可以在A4988,V8825,L293,298使用




問題及後續發展

問題:
用millis計算時間,所以機器會遇到不能連續用5天以上
馬達運動時有點卡頓現像:

    DISABLE LCD DISPLAY時可以解除這個像
    可能是CPU計算時間的問題
    可以在MEGA2560 試看看

後續發展:

    加上開始的按鍵
    加上home sensor ,每個LOOP結束後都做HOME的校正
    由通迅取代可變電阻

LINKS:
https://github.com/zzzrrr/EV-03-1
