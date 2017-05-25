package ltm;
public final class LTMTimestamp implements java.io.Serializable {
  public long sec_;
  public int usec_;

  public LTMTimestamp() {}

  public LTMTimestamp(long _sec_, int _usec_) {
    sec_ = _sec_;
    usec_ = _usec_;
  }
}
