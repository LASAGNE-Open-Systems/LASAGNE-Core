package ltm;
public final class LTMTopicDetails implements java.io.Serializable {
  public ltm.LTMTimestamp topicTime_;
  public int topicID_;
  public String topicData_;

  public LTMTopicDetails() {}

  public LTMTopicDetails(ltm.LTMTimestamp _topicTime_, int _topicID_, String _topicData_) {
    topicTime_ = _topicTime_;
    topicID_ = _topicID_;
    topicData_ = _topicData_;
  }
}
